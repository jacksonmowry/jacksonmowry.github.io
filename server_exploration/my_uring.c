#include <asm-generic/socket.h>
#include <assert.h>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080
#define MAX_WORK 256
#define BUFFER_SIZE 1024
#define MAX_FILES 1024
#define MAX_SQE_PER_LOOP 2

struct io_uring ring;

enum event_type { ACCEPT, READ, WRITE, CLOSE };

struct request {
    enum event_type type;
    int socket;
    char buffer[BUFFER_SIZE];
};

void signal_handler(int num) { io_uring_queue_exit(&ring); }

int add_accept_request(int listen_socket, struct sockaddr_in* client_addr,
                       socklen_t* client_addr_len) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_multishot_accept(
        sqe, listen_socket, (struct sockaddr*)client_addr, client_addr_len, 0);
    sqe->file_index = IORING_FILE_INDEX_ALLOC;
    struct request* req = malloc(sizeof(struct request));
    req->type = ACCEPT;
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int add_read_request(int socket) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    struct request* req = malloc(sizeof(struct request));
    req->type = READ;
    req->socket = socket;
    memset(req->buffer, 0, BUFFER_SIZE);

    io_uring_prep_recv(sqe, socket, req->buffer, BUFFER_SIZE, 0);
    io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int add_write_request(struct request* req, int len) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    req->type = WRITE;
    io_uring_prep_send(sqe, req->socket, req->buffer, len, 0);
    io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int add_close_request(int socket) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    struct request* req = malloc(sizeof(struct request));
    req->type = CLOSE;
    req->socket = socket;
    io_uring_prep_close_direct(sqe, socket);
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int main() {
    signal(SIGINT, signal_handler);

    int files[MAX_FILES];
    memset(files, -1, sizeof(files));

    io_uring_queue_init(MAX_WORK, &ring, 0);
    io_uring_register_files(&ring, files, MAX_FILES);

    /*****************************/
    /* setup server/input socket */
    /*****************************/
    struct sockaddr_in addr;
    int listen_socket = socket(PF_INET, SOCK_STREAM, 0);

    int enable = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &enable,
               sizeof(enable));
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEPORT, &enable,
               sizeof(enable));

    memset(&addr, 0, sizeof(addr));
    addr = (struct sockaddr_in){.sin_family = AF_INET,
                                .sin_port = htons(PORT),
                                .sin_addr.s_addr = htonl(INADDR_ANY)};

    int code = bind(listen_socket, (const struct sockaddr*)&addr, sizeof(addr));
    assert(code != -1);

    code = listen(listen_socket, MAX_WORK);
    assert(code != -1);

    /********************/
    /* main server loop */
    /********************/
    struct io_uring_cqe* cqe;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    add_accept_request(listen_socket, &client_addr, &client_addr_len);
    io_uring_submit(&ring);

    while (true) {
        int submissions = 0;
        int code = io_uring_wait_cqe(&ring, &cqe);

        while (true) {
            if (code == -EAGAIN || code == -EINTR) {
                break;
            }

            assert(code >= 0);

            struct request* req = (struct request*)cqe->user_data;
            switch (req->type) {
            case ACCEPT:
                if ((cqe->flags & IORING_CQE_F_MORE) == 0) {
                    free(req);
                    add_accept_request(listen_socket, &client_addr,
                                       &client_addr_len);
                }
                add_read_request(cqe->res);
                submissions += 1;
                break;
            case READ:
                if (cqe->res <= 0) {
                    add_close_request(req->socket);
                    submissions += 1;

                    free(req);
                    break;
                }
                add_write_request(req, cqe->res);
                add_read_request(req->socket);
                submissions += 2;
                break;
            case WRITE:
                free(req);
                break;
            case CLOSE:
                free(req);
                break;
            default:
                break;
            }

            io_uring_cqe_seen(&ring, cqe);
            if (io_uring_sq_space_left(&ring) < MAX_SQE_PER_LOOP) {
                break;
            }
            code = io_uring_peek_cqe(&ring, &cqe);
        }

        if (submissions > 0) {
            io_uring_submit(&ring);
        }
    }
}
