#include <argp.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define MAX_WORK 256
#define BUFFER_SIZE 1024
#define MAX_FILES 1024
#define MAX_SQE_PER_LOOP 2

enum event_type { ACCEPT, READ, WRITE, CLOSE };

int async_sqes = 0;
int batch = 0;
int fixed_files = 0;
int sq_poll = 0;
int multishot = 0;
int debug = 0;

struct request {
    enum event_type type;
    int socket;
    char buffer[BUFFER_SIZE];
};

struct io_uring ring;

void signal_handler(int num) {
    printf("Exiting\n");
    io_uring_queue_exit(&ring);
    exit(0);
}

void fatal_error(const char* syscall) {
    perror(syscall);
    exit(1);
}

int add_accept_request(int listen_socket, struct sockaddr_in* client_addr,
                       socklen_t* client_addr_len) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    if (multishot) {
        io_uring_prep_multishot_accept(sqe, listen_socket,
                                       (struct sockaddr*)client_addr,
                                       client_addr_len, 0);
    } else {
        io_uring_prep_accept(sqe, listen_socket, (struct sockaddr*)client_addr,
                             client_addr_len, 0);
    }
    if (fixed_files) {
        sqe->file_index = IORING_FILE_INDEX_ALLOC;
    }
    struct request* req = malloc(sizeof(struct request));
    req->type = ACCEPT;
    io_uring_sqe_set_data(sqe, req);
    if (async_sqes) {
        io_uring_sqe_set_flags(sqe, IOSQE_ASYNC);
    }

    return 0;
}

int add_read_request(int socket) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    struct request* req = malloc(sizeof(struct request));
    req->type = READ;
    req->socket = socket;
    memset(req->buffer, 0, BUFFER_SIZE);

    io_uring_prep_recv(sqe, socket, req->buffer, BUFFER_SIZE, 0);
    if (fixed_files) {
        io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);
    }
    io_uring_sqe_set_data(sqe, req);
    if (async_sqes) {
        io_uring_sqe_set_flags(sqe, IOSQE_ASYNC);
    }

    return 0;
}

int add_write_request(struct request* req, int len) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    req->type = WRITE;
    io_uring_prep_send(sqe, req->socket, req->buffer, len, 0);
    if (fixed_files) {
        io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);
    }
    io_uring_sqe_set_data(sqe, req);
    if (async_sqes) {
        io_uring_sqe_set_flags(sqe, IOSQE_ASYNC);
    }

    return 0;
}

int add_close_request(int socket) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    struct request* req = malloc(sizeof(struct request));
    req->type = CLOSE;
    req->socket = socket;
    if (fixed_files) {
        io_uring_prep_close_direct(sqe, socket);
    } else {
        io_uring_prep_close(sqe, socket);
    }

    io_uring_sqe_set_data(sqe, req);
    if (async_sqes) {
        io_uring_sqe_set_flags(sqe, IOSQE_ASYNC);
    }

    return 0;
}

void main_loop(int listen_socket) {
    struct io_uring_cqe* cqe;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if (multishot) {
        add_accept_request(listen_socket, &client_addr, &client_addr_len);
    } else {
        unsigned space = io_uring_sq_space_left(&ring);
        while (space > 0) {
            add_accept_request(listen_socket, &client_addr, &client_addr_len);
            space--;
        }
    }
    io_uring_submit(&ring);

    while (1) {
        int submissions = 0;

        int ret = io_uring_wait_cqe(&ring, &cqe);
        while (1) {
            if (ret == -EAGAIN || ret == -EINTR) {
                break;
            }

            if (ret < 0) {
                fatal_error("io_uring_wait_cqe");
            }

            struct request* req = (struct request*)cqe->user_data;
            if (cqe->res < 0) {
                fprintf(stderr, "Async request failed: %s for event: %d\n",
                        strerror(-cqe->res), req->type);
                exit(1);
            }

            switch (req->type) {
            case ACCEPT:
                if (debug > 1)
                    fprintf(stderr, "ACCEPT %d%s\n", cqe->res,
                            cqe->flags & IORING_CQE_F_MORE ? " (more)" : "");

                if (!multishot || (cqe->flags & IORING_CQE_F_MORE) == 0) {
                    if (debug > 2)
                        fprintf(stderr, "Adding accept request\n");
                    free(req);
                    add_accept_request(listen_socket, &client_addr,
                                       &client_addr_len);
                    submissions += 1;
                }
                add_read_request(cqe->res);
                submissions += 1;
                break;
            case READ:
                if (debug > 1)
                    fprintf(stderr, "READ %d\n", cqe->res);
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
                if (debug > 1)
                    fprintf(stderr, "WRITE %d\n", cqe->res);
                free(req);
                break;
            case CLOSE:
                if (debug > 1)
                    fprintf(stderr, "CLOSE %d returned %d\n", req->socket,
                            cqe->res);
                free(req);
                break;
            default:
                fprintf(stderr, "Unexpected req type %d\n", req->type);
                break;
            }

            io_uring_cqe_seen(&ring, cqe);
            if (!batch || io_uring_sq_space_left(&ring) < MAX_SQE_PER_LOOP) {
                break;
            }

            if (batch) {
                ret = io_uring_peek_cqe(&ring, &cqe);
            }
        }

        if (submissions > 0) {
            if (debug)
                fprintf(stderr, "Submitting %d SQEs\n", submissions);
            io_uring_submit(&ring);
        }
    }
}

int create_listen(int port) {
    int sock;
    struct sockaddr_in addr;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock != -1);

    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) <
        0) {
        fatal_error("setsockopt(SO_REUSEADDR)");
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable)) <
        0) {
        fatal_error("setsockopt(SO_REUSEPORT)");
    }

    memset(&addr, 0, sizeof(addr));
    addr = (struct sockaddr_in){.sin_family = AF_INET,
                                .sin_port = htons(port),
                                .sin_addr.s_addr = htonl(INADDR_ANY)};

    if (bind(sock, (const struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fatal_error("bind()");
    }

    if (listen(sock, MAX_WORK) < 0) {
        fatal_error("listen()");
    }

    return sock;
}

const char argp_program_doc[] = "Simple echo server\n"
                                "\n"
                                "Usage: ./simple_echo_server\n";

static const struct argp_option opts[] = {
    {"async", 'a', 0, 0, "Submit async requests"},
    {"batch", 'b', 0, 0, "Batch available work into single submission"},
    {"fixed", 'f', 0, 0, "Use pre-allocated fixed files"},
    {"multishot", 'm', 0, 0, "Use multishot accept requests"},
    {"sqpoll", 'p', 0, 0, "Use submission queue polling in the kernel"},
    {"debug", 'd', 0, 0, "Provide debug output, repeat for more verbose debug"},
    {},
};

error_t parse_opts(int key, char* arg, struct argp_state* state) {
    switch (key) {
    case 'a':
        async_sqes = 1;
        break;
    case 'b':
        batch = 1;
        break;
    case 'f':
        fixed_files = 1;
        break;
    case 'm':
        multishot = 1;
        break;
    case 'p':
        sq_poll = 1;
        break;
    case 'd':
        debug += 1;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static const struct argp argp = {
    .options = opts,
    .parser = parse_opts,
    .doc = argp_program_doc,
};

int main(int argc, char** argv) {
    int err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
    if (err) {
        return 0;
    }

    signal(SIGINT, signal_handler);

    int files[MAX_FILES];
    memset(files, -1, sizeof(files));

    io_uring_queue_init(MAX_WORK, &ring, sq_poll ? IORING_SETUP_SQPOLL : 0);
    if (fixed_files) {
        if (io_uring_register_files(&ring, files, MAX_FILES) < 0) {
            fatal_error("io_uring_register_files");
        }
    }

    int listen_socket = create_listen(PORT);
    main_loop(listen_socket);

    return 0;
}
