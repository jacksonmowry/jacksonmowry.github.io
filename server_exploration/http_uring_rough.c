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
#define MAX_WORK 1024
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

const char header[] =
    "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
size_t header_len = sizeof(header);

struct io_uring ring;

enum event_type {
    ACCEPT,
    READ,
    WRITE,
    CLOSE,
    OPEN,
    SPLICE,
    STATX,
    SEND,
    DEFAULT
};

struct request {
    enum event_type type;
    int socket;
    char buffer[BUFFER_SIZE];
    int pipes[2];
    struct statx stat;
    char* file_path;
    int file_fd;
};

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
    io_uring_sqe_set_data(sqe, req);

    return 0;
}

int add_write_request(struct request* req, int len) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    req->type = WRITE;
    char* method = strtok(req->buffer, " ");
    char* path = strtok(NULL, " ");
    req->file_path = path;

    // Get file client needs
    if (path[strlen(path) - 1] == '/') {
        // Dir listings are currently unimplemented
        assert("Received request for a dir" == false);
    } else {
        io_uring_prep_statx(sqe, AT_FDCWD, req->file_path + 1, 0, STATX_SIZE,
                            &(req->stat));
        req->type = STATX;
        io_uring_sqe_set_data(sqe, req);
    }
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

    return 0;
}

int add_header_send_request(struct request* req) {
    struct request* new_req = malloc(sizeof(*req));
    memcpy(new_req, req, sizeof(*req));
    new_req->type = SEND;
    size_t header_length =
        snprintf(new_req->buffer, sizeof(new_req->buffer) - 1,
                 "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: "
                 "text/plain\r\nContent-Length: "
                 "%llu\r\nTags: \"hi mom\"\r\n\r\n",
                 new_req->stat.stx_size);
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_send(sqe, new_req->socket, new_req->buffer, header_length, 0);
    sqe->flags |= IOSQE_IO_LINK;
    io_uring_sqe_set_data(sqe, new_req);

    return 0;
}

int add_splice_request(struct request* req) {
    if (pipe(req->pipes) != 0) {
        perror("pipe");
        assert("pipe failed" == false);
    }
    int pipe_sz = fcntl(req->pipes[0], F_SETPIPE_SZ, 4194304);
    pipe_sz = fcntl(req->pipes[1], F_SETPIPE_SZ, 4194304);
    /* printf("%d->%d->%d->%d\n", req->file_fd, req->pipes[1], req->pipes[0], */
    /*        req->socket); */

    loff_t off_in = 0;
    loff_t off_out = 0;

    // Splice pt. 1 file->pipe[1]
    struct request* req1 = malloc(sizeof(struct request));
    memcpy(req1, req, sizeof(*req));
    req1->type = SPLICE;

    off_in = 0;
    off_out = -1;
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    sqe->opcode = IORING_OP_SPLICE;
    io_uring_prep_splice(sqe, req1->file_fd, off_in, req1->pipes[1], off_out,
                         req1->stat.stx_size, 0 /*x01*/);
    sqe->flags |= IOSQE_IO_LINK;
    io_uring_sqe_set_data(sqe, req1);

    // Splice pt. 2 pipe[0]->socket
    struct request* req2 = malloc(sizeof(struct request));
    memcpy(req2, req, sizeof(*req));
    req2->type = SPLICE;

    off_in = -1;
    off_out = -1;
    sqe = io_uring_get_sqe(&ring);
    sqe->opcode = IORING_OP_SPLICE;
    io_uring_prep_splice(sqe, req2->pipes[0], off_in, req2->socket, off_out,
                         req2->stat.stx_size, 0 /*x01*/);
    sqe->flags |= IOSQE_IO_LINK;
    io_uring_sqe_set_data(sqe, req2);

    return 0;
}

int add_close_file_request(struct request* req) {
    struct request* new_req = malloc(sizeof(*req));
    memcpy(new_req, req, sizeof(*req));
    new_req->type = DEFAULT;
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close(sqe, req->file_fd);
    sqe->flags |= IOSQE_IO_LINK;
    io_uring_sqe_set_data(sqe, new_req);
    return 0;
}

int add_close_request(struct request* req) {
    struct request* new_req = malloc(sizeof(*req));
    memcpy(new_req, req, sizeof(*req));
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    new_req->type = CLOSE;
    io_uring_prep_close(sqe, new_req->socket);
    io_uring_sqe_set_data(sqe, new_req);

    return 0;
}

int main() {
    signal(SIGINT, signal_handler);

    io_uring_queue_init(4096, &ring, 0);

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
        int code = io_uring_wait_cqe(&ring, &cqe);

        if (code == -EAGAIN || code == -EINTR) {
            continue;
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
            break;
        case READ:
            if (cqe->res == 0) {
                add_close_request(req);
                free(req);
                break;
            }
            if (cqe->res < 0) {
                fprintf(stderr, "read: %s\n", strerror(-cqe->res));
                add_close_request(req);

                free(req);
                break;
            }
            add_write_request(req, cqe->res);
            break;
        case STATX:
            if (cqe->res < 0) {
                fprintf(stderr, "io_uring_prep_stat: %s\n",
                        strerror(-cqe->res));
                free(req);

                break;
            }
            add_open_request(req);
            break;
        case OPEN:
            if (cqe->res < 0) {
                printf("openat2: %s\n", strerror(-cqe->res));
                /* add_close_request(req->socket, NULL, req->file_fd); */
                free(req);
                assert("open failed" == false);
                break;
            }
            req->file_fd = cqe->res;

            add_header_send_request(req);
            add_splice_request(req); // has 2 internal submissions
            add_close_file_request(req);
            add_close_request(req);
            free(req);
            break;
        case SEND:
            if (cqe->res < 0) {
                fprintf(stderr, "Error: send() failed. Reason[%d]: %s\n",
                        -cqe->res, strerror(-cqe->res));
            }
            free(req);
            break;
        case SPLICE:
            if (cqe->res < 0) {
                fprintf(stderr, "Error: splice() failed. Reason[%d]: %s\n",
                        -cqe->res, strerror(-cqe->res));
                fprintf(stderr, "Failed on %d->%d->%d->%d\n", req->file_fd,
                        req->pipes[1], req->pipes[0], req->socket);
                assert("splice failed" == false);
            }
            free(req);
            break;
        case WRITE:
            free(req);
            break;
        case CLOSE:
            if (cqe->res < 0) {
                break;
            }
            if (close(req->pipes[0]) == -1) {
                assert("pipe[0] close failed");
            }
            if (close(req->pipes[1]) == -1) {
                assert("pipe[1] close failed");
            }
            free(req);
            break;
        default:
            if (cqe->res < 0) {
                fprintf(stderr, "Error: default() failed. Reason[%d]: %s\n",
                        -cqe->res, strerror(-cqe->res));
                assert("default failed" == false);
            }
            free(req);
            break;
        }

        io_uring_cqe_seen(&ring, cqe);

        io_uring_submit(&ring);
    }
}
