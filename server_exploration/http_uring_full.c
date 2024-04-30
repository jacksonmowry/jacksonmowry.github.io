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
#define MAX_WORK 1000000
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
    int file_fd;
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
    request* new_req = malloc(sizeof(*req));
    if (!new_req) {
        perror("malloc");
        exit(1);
    }
    memcpy(new_req, req, sizeof(*req));
    new_req->type = SEND;
    size_t header_length =
        snprintf(new_req->buffer, sizeof(new_req->buffer) - 1,
                 "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: "
                 "text/plain\r\nContent-Length: %llu\r\n\r\n",
                 new_req->stat.stx_size);

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_send(sqe, new_req->socket, new_req->buffer, header_length, 0);
    sqe->flags |= IOSQE_IO_LINK;
    io_uring_sqe_set_data(sqe, new_req);

    return 0;
}

int add_splice_request(request* req) {
    if (pipe(req->pipes) != 0) {
        perror("pipe");
        exit(1);
    }
    int pipe_sz = fcntl(req->pipes[0], F_SETPIPE_SZ, req->stat.stx_size);
    pipe_sz = fcntl(req->pipes[1], F_SETPIPE_SZ, req->stat.stx_size);

    request* req1 = malloc(sizeof(struct request));
    if (!req1) {
        perror("malloc");
        exit(1);
    }
    memcpy(req1, req, sizeof(*req));
    req1->type = SPLICE;

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_splice(sqe, req1->file_fd, 0, req1->pipes[1], -1,
                         req1->stat.stx_size, 0);
    sqe->flags |= IOSQE_IO_LINK;
    io_uring_sqe_set_data(sqe, req1);

    request* req2 = malloc(sizeof(struct request));
    if (!req2) {
        perror("malloc");
        exit(1);
    }
    memcpy(req2, req, sizeof(*req));
    req2->type = SPLICE;

    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_splice(sqe, req2->pipes[0], -1, req2->socket, -1,
                         req2->stat.stx_size, 0);
    sqe->flags |= IOSQE_IO_LINK;
    io_uring_sqe_set_data(sqe, req2);

    return 0;
}

int add_close_file_request(request* req) {
    request* new_req = malloc(sizeof(*req));
    if (!new_req) {
        perror("malloc");
        exit(1);
    }
    memcpy(new_req, req, sizeof(*req));
    new_req->type = CLOSE_FILE;

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close(sqe, req->file_fd);
    sqe->flags |= IOSQE_IO_LINK;
    io_uring_sqe_set_data(sqe, new_req);

    return 0;
}

int add_close_socket_request(request* req, bool link) {
    request* new_req = malloc(sizeof(*req));
    if (!new_req) {
        perror("malloc");
        exit(1);
    }
    memcpy(new_req, req, sizeof(*req));
    new_req->type = CLOSE_SOCKET;

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close(sqe, req->socket);
    if (link) {
        sqe->flags |= IOSQE_IO_LINK;
    }
    io_uring_sqe_set_data(sqe, new_req);

    return 0;
}

int add_close_pipe_request(request* req) {
    request* req_pipe1 = malloc(sizeof(*req));
    if (!req_pipe1) {
        perror("malloc");
        exit(1);
    }
    memcpy(req_pipe1, req, sizeof(*req));
    req_pipe1->type = CLOSE_PIPE;

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close(sqe, req_pipe1->pipes[0]);
    sqe->flags |= IOSQE_IO_LINK;
    io_uring_sqe_set_data(sqe, req_pipe1);

    request* req_pipe2 = malloc(sizeof(*req));
    if (!req_pipe2) {
        perror("malloc");
        exit(1);
    }
    memcpy(req_pipe2, req, sizeof(*req));
    req_pipe2->type = CLOSE_PIPE;

    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close(sqe, req_pipe2->pipes[1]);
    io_uring_sqe_set_data(sqe, req_pipe2);

    return 0;
}

int main() {
    signal(SIGINT, signal_handler);

    io_uring_queue_init(MAX_WORK, &ring, 0);
    /* io_uring_queue_init(MAX_WORK, &ring, IORING_SETUP_SQPOLL); */

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
            /* fprintf(stderr, "io_uring_wait_cqe: %s\n", strerror(-err)); */
            exit(1);
        }
        request* req = (request*)cqe->user_data;
        switch (req->type) {
        case ACCEPT:
            if ((cqe->flags & IORING_CQE_F_MORE) == 0) {
                free(req);
                add_accept_request(listen_socket, &client_addr,
                                   &client_addr_len);
            }

            add_read_request(cqe->res);
            break;
        case READ:
            if (cqe->res == 0) {
                add_close_socket_request(req, false);
                free(req);
                break;
            }
            if (cqe->res <= 0) {
                fprintf(stderr, "io_uring_prep_read: %s\n",
                        strerror(-cqe->res));
                add_close_socket_request(req, true);

                free(req);
                break;
            }

            add_statx_request(req);
            break;
        case STATX:
            if (cqe->res < 0) {
                fprintf(stderr, "io_uring_prep_statx: %s\n",
                        strerror(-cqe->res));
                add_close_socket_request(req, true);

                free(req);
                break;
            }

            add_open_request(req);
            break;
        case OPEN:
            if (cqe->res < 0) {
                fprintf(stderr, "io_uring_prep_openat2: %s\n",
                        strerror(-cqe->res));
                fprintf(stderr, "I was told to open %s\n", req->file_path + 1);
                add_close_socket_request(req, true);

                free(req);
                break;
            }

            req->file_fd = cqe->res;

            add_header_send_request(req);
            add_splice_request(req);
            add_close_file_request(req);
            add_close_socket_request(req, true);
            add_close_pipe_request(req);

            free(req);
            break;
        case SEND:
            if (cqe->res < 0) {
                fprintf(stderr, "io_uring_prep_send: %s\n",
                        strerror(-cqe->res));
                add_close_socket_request(req, true);

                free(req);
                break;
            }

            free(req);
            break;
        case SPLICE:
            if (cqe->res < 0) {
                fprintf(stderr, "io_uring_prep_splice: %s\n",
                        strerror(-cqe->res));
                add_close_socket_request(req, true);

                free(req);
                break;
            }

            free(req);
            break;
        case CLOSE_FILE:
            if (cqe->res < 0) {
                fprintf(stderr, "io_uring_prep_close: %s\n",
                        strerror(-cqe->res));
                add_close_socket_request(req, true);

                free(req);
                break;
            }

            free(req);
            break;
        case CLOSE_SOCKET:
            if (cqe->res < 0) {
                fprintf(stderr, "io_uring_prep_close: %s\n",
                        strerror(-cqe->res));

                free(req);
                break;
            }

            free(req);
            break;
        case CLOSE_PIPE:
            if (cqe->res < 0) {
                fprintf(stderr, "io_uring_prep_close: %s\n",
                        strerror(-cqe->res));
                add_close_socket_request(req, true);

                free(req);
                break;
            }
            free(req);
            break;
        default:
            assert("why are we in default" == false);
            break;
        }

        io_uring_cqe_seen(&ring, cqe);

        io_uring_submit(&ring);
    }
}
