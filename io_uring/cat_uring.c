#include <asm/unistd_64.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <linux/io_uring.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>

#define QUEUE_DEPTH 1
#define BLOCK_SZ 1024

#define read_barrier() __asm__ __volatile__("" ::: "memory")
#define write_barrier() __asm__ __volatile__("" ::: "memory")

struct app_io_sq_ring {
    __u32* head;
    __u32* tail;
    __u32* ring_mask;
    __u32* ring_entries;
    __u32* flags;
    __u32* array;
};

struct app_io_cq_ring {
    __u32* head;
    __u32* tail;
    __u32* ring_mask;
    __u32* ring_entires;
    struct io_uring_cqe* cqes;
};

struct submitter {
    int ring_fd;
    struct app_io_sq_ring sq_ring;
    struct io_uring_sqe* sqes;
    struct app_io_cq_ring cq_ring;
};

struct file_info {
    off_t file_sz;
    struct iovec iovecs[];
};

int io_uring_setup(__u32 entries, struct io_uring_params* p) {
    return (__s32)syscall(__NR_io_uring_setup, entries, p);
}

int io_uring_enter(int ring_fd, __u32 to_submit, __u32 min_complete,
                   __u32 flags) {
    return (__s32)syscall(__NR_io_uring_enter, ring_fd, to_submit, min_complete,
                          flags, NULL, 0);
}

off_t get_file_size(int fd) {
    struct stat st;

    if (fstat(fd, &st) < 0) {
        perror("fstat");
        return -1;
    }
    if (S_ISBLK(st.st_mode)) {
        __u64 bytes;
        if (ioctl(fd, BLKGETSIZE64, &bytes) != 0) {
            perror("ioctl");
            return -1;
        }
        return bytes;
    } else if (S_ISREG(st.st_mode)) {
        return st.st_size;
    }

    return -1;
}

__s32 app_setup_uring(struct submitter* s) {
    struct app_io_sq_ring* sring = &s->sq_ring;
    struct app_io_cq_ring* cring = &s->cq_ring;
    struct io_uring_params p;
    void* sq_ptr;
    void* cq_ptr;

    memset(&p, 0, sizeof(p));
    s->ring_fd = io_uring_setup(QUEUE_DEPTH, &p);
    if (s->ring_fd < 0) {
        perror("io_uring_setup");
        return 1;
    }

    int sring_sz = p.sq_off.array + p.sq_entries * sizeof(__u32);
    int cring_sz = p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);

    if (p.features & IORING_FEAT_SINGLE_MMAP) {
        if (cring_sz > sring_sz) {
            sring_sz = cring_sz;
        }
        cring_sz = sring_sz;
    }

    sq_ptr = mmap(0, sring_sz, PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_POPULATE, s->ring_fd, IORING_OFF_SQ_RING);
    if (sq_ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    if (p.features & IORING_FEAT_SINGLE_MMAP) {
        cq_ptr = sq_ptr;
    } else {
        cq_ptr =
            mmap(0, cring_sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
                 s->ring_fd, IORING_OFF_CQ_RING);
        if (cq_ptr == MAP_FAILED) {
            perror("mmap");
            return 1;
        }
    }

    sring->head = sq_ptr + p.sq_off.head;
    sring->tail = sq_ptr + p.sq_off.tail;
    sring->ring_mask = sq_ptr + p.sq_off.ring_mask;
    sring->ring_entries = sq_ptr + p.sq_off.ring_entries;
    sring->flags = sq_ptr + p.sq_off.flags;
    sring->array = sq_ptr + p.sq_off.array;

    s->sqes = mmap(0, p.sq_entries * sizeof(struct io_uring_sqe),
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
                   s->ring_fd, IORING_OFF_SQES);
    if (s->sqes == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    cring->head = cq_ptr + p.cq_off.head;
    cring->tail = cq_ptr + p.cq_off.tail;
    cring->ring_mask = cq_ptr + p.cq_off.ring_mask;
    cring->ring_entires = cq_ptr + p.cq_off.ring_entries;
    cring->cqes = cq_ptr + p.cq_off.cqes;

    return 0;
}

void output_to_console(char* buf, __s32 len) {
    while (len--) {
        fputc(*buf++, stdout);
    }
}

void read_from_cq(struct submitter* s) {
    struct file_info* fi;
    struct app_io_cq_ring* cring = &s->cq_ring;
    struct io_uring_cqe* cqe;
    __u32 head;
    __u32 reaped = 0;

    head = *cring->head;

    do {
        read_barrier();

        if (head == *cring->tail) {
            break;
        }

        cqe = &cring->cqes[head & *s->cq_ring.ring_mask];
        fi = (struct file_info*)cqe->user_data;
        if (cqe->res < 0) {
            fprintf(stderr, "Error: %s\n", strerror(abs(cqe->res)));
        }

        int blocks = (__s32)fi->file_sz / BLOCK_SZ;
        if (fi->file_sz % BLOCK_SZ) {
            blocks++;
        }

        for (__s32 i = 0; i < blocks; i++) {
            output_to_console(fi->iovecs[i].iov_base, fi->iovecs[i].iov_len);
        }

        head++;
    } while (1);

    *cring->head = head;
    write_barrier();
}

__s32 submit_to_sq(char* file_path, struct submitter* s) {
    struct file_info* fi;

    __s32 file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        perror("open");
        return 1;
    }

    struct app_io_sq_ring* sring = &s->sq_ring;
    __u32 index = 0;
    __u32 current_block = 0;
    __u32 tail = 0;
    __u32 next_tail = 0;

    off_t file_sz = get_file_size(file_fd);
    if (file_sz < 0) {
        return 1;
    }

    off_t bytes_remaining = file_sz;
    int blocks = (__s32)file_sz / BLOCK_SZ;
    if (file_sz % BLOCK_SZ) {
        blocks++;
    }

    fi = malloc(sizeof(*fi) + (sizeof(struct iovec) * blocks));
    if (!fi) {
        fprintf(stderr, "Unable to allocate memory\n");
        return 1;
    }
    fi->file_sz = file_sz;

    while (bytes_remaining) {
        off_t bytes_to_read = bytes_remaining;
        if (bytes_to_read > BLOCK_SZ) {
            bytes_to_read = BLOCK_SZ;
        }

        fi->iovecs[current_block].iov_len = bytes_to_read;

        void* buf;
        if ((buf = malloc(BLOCK_SZ))) {
            perror("malloc");
            return 1;
        }
        fi->iovecs[current_block].iov_base = buf;

        current_block++;
        bytes_remaining -= bytes_to_read;
    }

    next_tail = tail = *sring->tail;
    next_tail++;
    read_barrier();
    index = tail & *s->sq_ring.ring_mask;
    struct io_uring_sqe* sqe = &s->sqes[index];
    sqe->fd = file_fd;
    sqe->flags = 0;
    sqe->opcode = IORING_OP_READV;
    sqe->addr = (__u64)fi->iovecs;
    sqe->len = blocks;
    sqe->off = 0;
    sqe->user_data = (__u64)fi;
    sring->array[index] = index;
    tail = next_tail;

    if (*sring->tail != tail) {
        *sring->tail = tail;
        write_barrier();
    }

    __s32 ret = io_uring_enter(s->ring_fd, 1, 1, IORING_ENTER_GETEVENTS);
    if (ret < 0) {
        perror("io_uring_enter");
        return 1;
    }

    return 0;
}

__s32 main(__s32 argc, char** argv) {
    struct submitter* s;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    s = malloc(sizeof(*s));
    if (!s) {
        perror("malloc");
        return 1;
    }
    memset(s, 0, sizeof(*s));

    if (app_setup_uring(s)) {
        fprintf(stderr, "Unable to setup uring!\n");
        return 1;
    }

    for (__s32 i = 1; i < argc; i++) {
        if (submit_to_sq(argv[i], s)) {
            fprintf(stderr, "Error reading file\n");
            return 1;
        }
        read_from_cq(s);
    }

    return 0;
}
