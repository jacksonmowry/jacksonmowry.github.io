#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#define QUEUE_DEPTH 1
#define BLOCK_SZ 1024

struct file_info {
    off_t file_sz;
    struct iovec iovecs[];
};

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

void output_to_console(char* buf, __s32 len) {
    while (len--) {
        fputc(*buf++, stdout);
    }
}

__s32 get_completion_and_print(struct io_uring* ring) {
    struct io_uring_cqe* cqe;
    __s32 ret = io_uring_wait_cqe(ring, &cqe);
    if (ret < 0) {
        perror("io_uring_wait_cqe");
        return 1;
    }
    if (cqe->res < 0) {
        fprintf(stderr, "Async readv failed.\n");
        return 1;
    }
    struct file_info* file_info = io_uring_cqe_get_data(cqe);
    __s32 blocks = (__s32)file_info->file_sz / BLOCK_SZ;
    if (file_info->file_sz % BLOCK_SZ) {
        blocks++;
    }
    for (__s32 i = 0; i < blocks; i++) {
        output_to_console(file_info->iovecs[i].iov_base,
                          file_info->iovecs[i].iov_len);
    }

    io_uring_cqe_seen(ring, cqe);
    return 0;
}

__s32 submit_read_request(char* file_path, struct io_uring* ring) {
    __s32 file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        perror("open");
        return 1;
    }
    off_t file_sz = get_file_size(file_fd);
    off_t bytes_remaining = file_sz;
    off_t offset = 0;
    __s32 current_block = 0;
    __s32 blocks = (__s32)file_sz / BLOCK_SZ;
    if (file_sz % BLOCK_SZ) {
        blocks++;
    }
    struct file_info* file_info =
        malloc(sizeof(*file_info) + (sizeof(struct iovec) * blocks));

    while (bytes_remaining) {
        off_t bytes_to_read = bytes_remaining;
        if (bytes_to_read > BLOCK_SZ) {
            bytes_to_read = BLOCK_SZ;
        }

        offset += bytes_to_read;
        file_info->iovecs[current_block].iov_len = bytes_to_read;

        void* buf = malloc(BLOCK_SZ);
        if (!buf) {
            perror("malloc");
            return 1;
        }
        file_info->iovecs[current_block].iov_base = buf;

        current_block++;
        bytes_remaining -= bytes_to_read;
    }
    file_info->file_sz = file_sz;

    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    io_uring_prep_readv(sqe, file_fd, file_info->iovecs, blocks, 0);
    io_uring_sqe_set_data(sqe, file_info);
    io_uring_submit(ring);

    return 0;
}

__s32 main(__s32 argc, char** argv) {
    struct io_uring ring;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [file name] <[file name]...>\n", argv[0]);
        return 1;
    }

    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

    for (__s32 i = 1; i < argc; i++) {
        __s32 ret = submit_read_request(argv[i], &ring);
        if (ret) {
            fprintf(stderr, "Error reading file: %s\n", argv[i]);
            return 1;
        }
        get_completion_and_print(&ring);
    }

    io_uring_queue_exit(&ring);
    return 0;
}
