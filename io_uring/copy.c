#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <liburing.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#define QD 2
#define BS (16 * 1024)

static int infd, outfd;

typedef struct io_data {
    int32_t read;
    off_t first_offset;
    off_t offset;
    size_t first_len;
    struct iovec iov;
} io_data;

static int32_t setup_context(uint32_t entries, struct io_uring* ring) {
    if ((io_uring_queue_init(entries, ring, 0)) < 0) {
        perror("io_uring_queue_init");
        return -1;
    }

    return 0;
}

static int32_t get_file_size(int32_t fd, off_t* size) {
    struct stat st;

    if (fstat(fd, &st) < 0) {
        return -1;
    }
    if (S_ISREG(st.st_mode)) {
        *size = st.st_size;
        return 0;
    } else if (S_ISBLK(st.st_mode)) {
        uint64_t bytes;

        if (ioctl(fd, BLKGETSIZE64, &bytes) != 0) {
            return -1;
        }

        *size = bytes;
        return 0;
    }
    return -1;
}

static void queue_prepped(struct io_uring* ring, io_data* data) {
    struct io_uring_sqe* sqe;

    sqe = io_uring_get_sqe(ring);
    assert(sqe);

    if (data->read) {
        io_uring_prep_readv(sqe, infd, &data->iov, 1, data->offset);
    } else {
        io_uring_prep_writev(sqe, outfd, &data->iov, 1, data->offset);
    }

    io_uring_sqe_set_data(sqe, data);
}

static int32_t queue_read(struct io_uring* ring, off_t size, off_t offset) {
    struct io_uring_sqe* sqe;
    io_data* data;

    data = malloc(size + sizeof(*data));
    assert(data);

    sqe = io_uring_get_sqe(ring);
    assert(sqe);

    data->read = 1;
    data->offset = data->first_offset = offset;

    data->iov.iov_base = data + 1;
    data->iov.iov_len = size;
    data->first_len = size;

    io_uring_prep_readv(sqe, infd, &data->iov, 1, offset);
    io_uring_sqe_set_data(sqe, data);
    return 0;
}

static void queue_write(struct io_uring* ring, io_data* data) {
    data->read = 0;
    data->offset = data->first_offset;

    data->iov.iov_base = data + 1;
    data->iov.iov_len = data->first_len;

    queue_prepped(ring, data);
    io_uring_submit(ring);
}

int32_t copy_file(struct io_uring* ring, off_t insize) {
    uint64_t reads;
    uint64_t writes;
    struct io_uring_cqe* cqe;
    off_t write_left;
    off_t offset;
    int32_t ret;

    write_left = insize;
    writes = 0;
    reads = 0;
    offset = 0;

    while (insize || write_left) {
        int32_t had_reads, got_comp;

        // Queue up as many reads as we can
        had_reads = reads;
        while (insize) {
            off_t this_size = insize;

            if (reads + writes >= QD) {
                break;
            }
            if (this_size > BS) {
                this_size = BS;
            } else if (!this_size) {
                break;
            }

            if (queue_read(ring, this_size, offset)) {
                break;
            }

            insize -= this_size;
            offset += this_size;
            reads++;
        }

        if (had_reads != reads) {
            ret = io_uring_submit(ring);
            if (ret < 0) {
                perror("io_uring_submit");
                break;
            }
        }

        // Queue is full, find a completion
        got_comp = 0;
        while (write_left) {
            io_data* data;

            if (!got_comp) {
                ret = io_uring_wait_cqe(ring, &cqe);
                got_comp = 1;
            } else {
                ret = io_uring_peek_cqe(ring, &cqe);
                if (ret == -EAGAIN) {
                    cqe = NULL;
                    ret = 0;
                }
            }
            if (ret < 0) {
                perror("io_uring_peek_cqe");
                return 1;
            }
            if (!cqe) {
                break;
            }

            data = io_uring_cqe_get_data(cqe);
            if (cqe->res < 0) {
                if (cqe->res == -EAGAIN) {
                    queue_prepped(ring, data);
                    io_uring_cqe_seen(ring, cqe);
                    continue;
                }
                perror("io_uring_cqe_get_data");
                return 1;
            } else if (cqe->res != data->iov.iov_len) {
                // Short read/write; adjust and requeue
                data->iov.iov_base += cqe->res;
                data->iov.iov_len -= cqe->res;
                queue_prepped(ring, data);
                io_uring_cqe_seen(ring, cqe);
                continue;
            }

            if (data->read) {
                queue_write(ring, data);
                write_left -= data->first_len;
                reads--;
                writes++;
            } else {
                free(data);
                writes--;
            }
            io_uring_cqe_seen(ring, cqe);
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    struct io_uring ring;
    off_t insize;
    int32_t ret;

    if (argc < 3) {
        printf("Usage: %s <infile> <outfile>\n", argv[0]);
        return 1;
    }

    infd = open(argv[1], O_RDONLY);
    if (infd < 0) {
        perror("open infile");
        return 1;
    }

    outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outfd < 0) {
        perror("open outfile");
        return 1;
    }

    if (setup_context(QD, &ring)) {
        return 1;
    }

    if (get_file_size(infd, &insize)) {
        return 1;
    }

    ret = copy_file(&ring, insize);

    close(infd);
    close(outfd);
    io_uring_queue_exit(&ring);
    return ret;
}
