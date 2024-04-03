#include <assert.h>
#include <fcntl.h>
#include <liburing.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FILE_NAME "/tmp/io_uring_test.txt"
#define STR "Hello, io_uring!"
char buff[32];

int32_t link_operations(struct io_uring* ring) {
    struct io_uring_sqe* sqe;
    struct io_uring_cqe* cqe;

    int32_t fd = open(FILE_NAME, O_RDWR | O_TRUNC | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    sqe = io_uring_get_sqe(ring);
    assert(sqe);

    io_uring_prep_write(sqe, fd, STR, strlen(STR), 0);
    sqe->flags |= IOSQE_IO_LINK;

    sqe = io_uring_get_sqe(ring);
    assert(sqe);

    io_uring_prep_read(sqe, fd, buff, strlen(STR), 0);
    sqe->flags |= IOSQE_IO_LINK;

    sqe = io_uring_get_sqe(ring);
    assert(sqe);

    io_uring_prep_close(sqe, fd);

    io_uring_submit(ring);

    for (int32_t i = 0; i < 3; i++) {
        int32_t ret = io_uring_wait_cqe(ring, &cqe);
        if (ret < 0) {
            assert("io_uring_wait_cqe");
            return 1;
        }
        // We now have the cqe, lets process data
        if (cqe->res < 0) {
            fprintf(stderr, "Error in async operation: %s\n",
                    strerror(-cqe->res));
        }
        printf("Result of the operation: %d\n", cqe->res);
        io_uring_cqe_seen(ring, cqe);
    }

    printf("buffer contents: %s\n", buff);
    return 0;
}

int32_t main() {
    struct io_uring ring;

    int32_t ret = io_uring_queue_init(8, &ring, 0);
    if (ret) {
        fprintf(stderr, "Unable to setup io_uring : %s\n", strerror(-ret));
        return 1;
    }

    link_operations(&ring);
    io_uring_queue_exit(&ring);
    return 0;
}
