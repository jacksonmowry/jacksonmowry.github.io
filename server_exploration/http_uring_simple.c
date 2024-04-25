#include <asm-generic/socket.h>
#include <assert.h>
#include <bits/types/struct_iovec.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#define _GNU_SOURCE
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
#define MAX_WORK 2048
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

enum event_type { ACCEPT, READ, WRITE, CLOSE };

struct request {
    enum event_type type;
    int socket;
    char buffer[BUFFER_SIZE];
};

struct io_uring ring;
struct request* listening_request;

void signal_handler(int num) {
    io_uring_queue_exit(&ring);
    free(listening_request);
    printf("Cleaning up and exiting\n");
    exit(0);
}

int add_accept_request(int listen_socket, struct sockaddr_in* client_addr,
                       socklen_t* client_addr_len) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_accept(sqe, listen_socket, (struct sockaddr*)client_addr,
                         client_addr_len, 0);
    listening_request = malloc(sizeof(struct request));
    listening_request->type = ACCEPT;
    io_uring_sqe_set_data(sqe, listening_request);
    /* io_uring_submit(&ring); */

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
    /* io_uring_submit(&ring); */

    return 0;
}

int add_write_request(struct request* req) {
    if (!req->buffer[0]) {
        send(req->socket, http_404_content, strlen(http_404_content), 0);
        return 0;
    }

    char* method = strtok(req->buffer, " ");
    char* path = strtok(NULL, " "); // Skip the initial slash
    assert(path);
    ++path;
    char* final_path = realpath(path, NULL);

    if (!final_path) {
        send(req->socket, http_404_content, strlen(http_404_content), 0);
        return 0;
    }

    assert(final_path);
    struct stat st;
    int code = stat(final_path, &st);
    if (code == -1) {
        perror("stat");
        exit(1);
    }

    size_t header_length =
        snprintf(req->buffer, BUFFER_SIZE,
                 "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: "
                 "text/plain\r\nContent-Length: %ld\r\n\r\n",
                 st.st_size);
    code = send(req->socket, req->buffer, header_length, 0);
    if (code < 0) {
        free(final_path);

        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        req->type = CLOSE;
        io_uring_prep_close(sqe, req->socket);
        io_uring_sqe_set_data(sqe, req);
        io_uring_submit(&ring);

        return 0;
    }

    int fd = open(final_path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    free(final_path);
    code = sendfile(req->socket, fd, NULL, st.st_size);
    if (code < 0) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        req->type = CLOSE;
        io_uring_prep_close(sqe, req->socket);
        io_uring_sqe_set_data(sqe, req);
        io_uring_submit(&ring);

        return 0;
    }
    code = close(fd);
    if (code == -1) {
        perror("close");
        exit(1);
    }

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    req->type = CLOSE;
    io_uring_prep_close(sqe, req->socket);
    io_uring_sqe_set_data(sqe, req);
    /* io_uring_submit(&ring); */

    return 0;
}

int add_close_request(struct request* req) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    req->type = CLOSE;
    io_uring_prep_close(sqe, req->socket);
    io_uring_sqe_set_data(sqe, req);
    /* io_uring_submit(&ring); */

    return 0;
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    io_uring_queue_init(MAX_WORK, &ring, IORING_SETUP_SQPOLL);

    struct sockaddr_in addr = {0};
    int listen_socket = socket(PF_INET, SOCK_STREAM, 0);

    int enable = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &enable,
               sizeof(enable));
    /* setsockopt(listen_socket, SOL_SOCKET, SO_REUSEPORT, &enable,
     * sizeof(enable)); */
    addr = (struct sockaddr_in){.sin_family = AF_INET,
                                .sin_port = htons(PORT),
                                .sin_addr.s_addr = htonl(INADDR_ANY)};

    int code = bind(listen_socket, (const struct sockaddr*)&addr, sizeof(addr));
    if (code == -1) {
        perror("bind");
        exit(1);
    }

    code = listen(listen_socket, MAX_WORK);
    if (code == -1) {
        perror("listen");
        exit(1);
    }

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

        if (code < 0) {
            fprintf(stderr, "io_uring_wait_cqe: %s\n", strerror(-code));
            exit(1);
        }

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
                break;
            }
            if (cqe->res < 0) {
                if (-cqe->res == EBADF) {
                    free(req);
                    break;
                }
                fprintf(stderr, "%d: io_uring_prep_read: %s\n", cqe->res,
                        strerror(-cqe->res));
                add_close_request(req);

                break;
            }
            add_write_request(req);
            break;
        /* case WRITE: */
        /*     add_close_request(req); */
        /*     break; */
        case CLOSE:
            if (cqe->res < 0) {
                fprintf(stderr, "myclose: %s\n", strerror(-cqe->res));
                fprintf(stderr, "Failed closing %d\n", req->socket);
                exit(1);
            }
            free(req);
            break;
        default:
            free(req);
            break;
        }

        io_uring_cqe_seen(&ring, cqe);

        io_uring_submit(&ring);
    }
    printf("We should never get here\n");
}
