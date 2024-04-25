#define _GNU_SOURCE
#include <asm-generic/socket.h>
#include <assert.h>
#include <bits/types/struct_iovec.h>
#include <sys/socket.h>
#define _FILE_OFFSET_BITS 64
#include <fcntl.h>
#include <liburing.h>
#include <linux/stat.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define PORT 8069
#define MAX_WORK 8192
#define BUFFER_SIZE 1024
#define MAX_FILES 1024
#define MAX_SQE_PER_LOOP 2

const char* http_404_content = "HTTP/1.0 404 Not Found\r\n"
                               "Content-type: text/html\r\n"
                               "\r\n"
                               "<html>"
                               "<head>"
                               "<title>File Serve: Not Found</title>"
                               "</head>"
                               "<body>"
                               "<h1>Not Found (404)</h1>"
                               "<p>Your client is asking for an object that "
                               "was not found on this server.</p>"
                               "</body>"
                               "</html>";

struct io_uring ring;

typedef enum event_type {
    ACCEPT,
    READ,
    WRITE,
    CLOSE_FILE,
    CLOSE_SOCKET,
    CLOSE_PIPE,
    OPEN,
    SPLICE,
    STATX,
    SEND,
    DEFAULT,
} event_type;

typedef struct request {
    event_type type;
    int socket;
    char buffer[BUFFER_SIZE];
    char* file_path;
    struct statx stat;
    int pipes[2];
    bool splice_ready;
    int file_fd;
    bool error;
} request;

void signal_handler(int num) {
    io_uring_queue_exit(&ring);
    exit(0);
}

int add_accept_request(int listen_socket, struct sockaddr_in* client_addr,
                       socklen_t* client_addr_len) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_accept(sqe, listen_socket, (struct sockaddr*)client_addr,
                         client_addr_len, 0);
    struct request* req = malloc(sizeof(struct request));
    if (!req) {
        perror("malloc");
        exit(1);
    }
    req->type = ACCEPT;
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int add_read_request(int socket) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    struct request* req = malloc(sizeof(struct request));
    if (!req) {
        perror("malloc");
        exit(1);
    }
    req->type = READ;
    req->socket = socket;
    memset(req->buffer, 0, BUFFER_SIZE);

    io_uring_prep_recv(sqe, socket, req->buffer, BUFFER_SIZE, 0);
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int add_statx_request(struct request* req) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    req->type = STATX;

    char* method = strtok(req->buffer, " ");
    char* path = strtok(NULL, " ");
    req->file_path = path;

    io_uring_prep_statx(sqe, AT_FDCWD, req->file_path + 1, 0, STATX_SIZE,
                        &(req->stat));
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int add_open_request(struct request* req) {
    size_t file_size = req->stat.stx_size;
    struct open_how how;
    memset(&how, 0, sizeof(how));
    how.flags = O_RDONLY;

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_openat2(sqe, AT_FDCWD, req->file_path + 1, &how);
    io_uring_sqe_set_data(sqe, req);
    req->type = OPEN;
    io_uring_submit(&ring);

    return 0;
}

int add_header_send_request(request* req) {
    // puts("beginning header send");
    req->type = SEND;
    size_t header_length =
        snprintf(req->buffer, sizeof(req->buffer) - 1,
                 "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: "
                 "text/plain\r\nContent-Length: %llu\r\n\r\n",
                 req->stat.stx_size);

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_send(sqe, req->socket, req->buffer, header_length, 0);
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int add_splice_request(request* req) {
    if (pipe(req->pipes) != 0) {
        perror("pipe");
        exit(1);
    }

    req->type = SPLICE;

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_splice(sqe, req->file_fd, 0, req->pipes[1], -1,
                         req->stat.stx_size, 0);
    io_uring_sqe_set_data(sqe, req);

    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_splice(sqe, req->pipes[0], -1, req->socket, -1,
                         req->stat.stx_size, 0);
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int add_close_file_request(request* req) {
    req->type = CLOSE_FILE;

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close(sqe, req->file_fd);
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int add_close_socket_request(request* req, bool error) {
    req->type = CLOSE_SOCKET;

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close(sqe, req->socket);
    if (error) {
        request* new_req = malloc(sizeof(request));
        memcpy(new_req, req, sizeof(*req));
        req->error = true;
        io_uring_sqe_set_data(sqe, new_req);
    } else {
        io_uring_sqe_set_data(sqe, req);
    }

    return 0;
}

int add_close_pipe_request(request* req) {
    req->splice_ready = false;
    req->type = CLOSE_PIPE;
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close(sqe, req->pipes[0]);
    io_uring_sqe_set_data(sqe, req);

    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close(sqe, req->pipes[1]);
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int main() {
    signal(SIGINT, signal_handler);

    io_uring_queue_init(MAX_WORK, &ring, 0);

    struct sockaddr_in addr = {0};
    int listen_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_socket == -1) {
        perror("socket");
        exit(1);
    }

    int enable = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &enable,
               sizeof(enable));
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEPORT, &enable,
               sizeof(enable));

    addr = (struct sockaddr_in){.sin_family = AF_INET,
                                .sin_port = htons(PORT),
                                .sin_addr.s_addr = htonl(INADDR_ANY)};

    int err = bind(listen_socket, (const struct sockaddr*)&addr, sizeof(addr));
    if (err == -1) {
        perror("bind");
        exit(1);
    }

    err = listen(listen_socket, MAX_WORK);
    if (err == -1) {
        perror("listen");
        exit(1);
    }

    struct io_uring_cqe* cqe;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    add_accept_request(listen_socket, &client_addr, &client_addr_len);
    io_uring_submit(&ring);

    while (true) {
        int err = io_uring_wait_cqe(&ring, &cqe);

        if (err == -EAGAIN || err == -EINTR) {
            continue;
        }

        if (err < 0) {
            exit(1);
        }
        request* req = (request*)cqe->user_data;
        switch (req->type) {
        case ACCEPT:
            // puts("accept");
            if ((cqe->flags & IORING_CQE_F_MORE) == 0) {
                free(req);
                add_accept_request(listen_socket, &client_addr,
                                   &client_addr_len);
            }

            add_read_request(cqe->res);
            break;
        case READ:
            // puts("read");
            if (cqe->res == 0) {
                add_close_socket_request(req, true);
                free(req);
                break;
            }
            if (cqe->res <= 0) {

                free(req);
                break;
            }

            add_statx_request(req);
            break;
        case STATX:
            // puts("statx");
            if (cqe->res < 0) {

                // fprintf(stderr, "io_uring_prep_statx: %s\n",
                // strerror(-cqe->res));
                free(req);
                break;
            }

            add_open_request(req);
            break;
        case OPEN:
            // puts("open");
            if (cqe->res < 0) {
                // fprintf(stderr, "io_uring_prep_open: %s\n",
                // strerror(-cqe->res));
                add_close_socket_request(req, true);
                free(req);
                break;
            }

            req->file_fd = cqe->res;

            add_header_send_request(req);
            /* add_splice_request(req); */
            /* add_close_file_request(req); */
            /* add_close_socket_request(req, true); */
            /* add_close_pipe_request(req); */

            /* free(req); */
            break;
        case SEND:
            // puts("send");
            if (cqe->res < 0) {

                // fprintf(stderr, "io_uring_prep_send: %s\n",
                // strerror(-cqe->res));
                free(req);
                break;
            }

            add_splice_request(req);
            /* free(req); */
            break;
        case SPLICE:
            // puts("splice");
            if (cqe->res < 0) {
                // fprintf(stderr, "io_uring_prep_splice: %s\n",
                // strerror(-cqe->res));
                free(req);
                break;
            }
            if (req->splice_ready) {
                add_close_socket_request(req, false);
            } else {
                req->splice_ready = true;
            }
            /* free(req); */
            break;
        case CLOSE_FILE:
            // puts("close file");
            if (cqe->res < 0) {
                // fprintf(stderr, "io_uring_prep_close: %s\n",
                // strerror(-cqe->res));
                free(req);
                break;
            }

            add_close_pipe_request(req);
            /* free(req); */
            break;
        case CLOSE_SOCKET:
            // puts("close socket");
            if (cqe->res < 0) {

                // fprintf(stderr, "io_uring_prep_close: %s\n",
                // strerror(-cqe->res));
                free(req);
                break;
            }
            if (req->error) {
                free(req);
            } else {
                add_close_file_request(req);
            }
            /* free(req); */
            break;
        case CLOSE_PIPE:
            // puts("close pipe");
            if (cqe->res < 0) {

                // fprintf(stderr, "io_uring_prep_close: %s\n",
                // strerror(-cqe->res));
                free(req);
                break;
            }
            if (req->splice_ready) {
                free(req);
            } else {
                req->splice_ready = true;
            }
            break;
        default:
            assert("why are we in default" == false);
            break;
        }

        io_uring_cqe_seen(&ring, cqe);

        io_uring_submit(&ring);
    }
}
