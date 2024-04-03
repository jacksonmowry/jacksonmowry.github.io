#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080
#define MAX_CLIENTS 1024

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void* worker(int);
int accept_new_conn(int server_socket);
int setup_server(short port, int backlog);

int main(/* int argc, char** argv */) {
    char buffer[BUFFER_SIZE];
    bool server_running = true;
    bool close_conn;
    bool compress_array = false;
    int server_socket = setup_server(PORT, MAX_CLIENTS);
    int client_socket;

    struct pollfd fds[1024];
    int nfds = 1, current_size = 0;

    memset(fds, 0, sizeof(fds));

    fds[0].fd = server_socket;
    fds[0].events = POLLIN;

    do {
        int err = poll(fds, nfds, -1);
        assert(err >= 0);

        current_size = nfds;
        for (int i = 0; i < current_size; i++) {
            // Find FD with POLLIN
            if (fds[i].revents == 0) {
                continue;
            }

            // If not POLLIN, unexpected error
            /* if (fds[]) */

            if (fds[i].fd == server_socket) {
                // New connection to accept
                do {
                    // pull on connections off incoming queue
                    client_socket = accept(server_socket, NULL, NULL);
                    if (client_socket < 0) {
                        break;
                    }

                    fds[nfds].fd = client_socket;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } while (client_socket != -1);
            } else {
                // Not the listening socket, read from client
                close_conn = false;

                do {
                    int err = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    if (err < 0) {
                        break;
                    }

                    if (err == 0) {
                        // client closed
                        close_conn = true;
                        break;
                    }

                    err = send(fds[i].fd, buffer, err, 0);
                    if (err < 0) {
                        close_conn = true;
                        break;
                    }
                } while (true);

                if (close_conn) {
                    // close client conneciton
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = true;
                }
            }
        }

        if (compress_array) {
            // remove empty space in list of fds to poll
            compress_array = false;
            for (int i = 0; i < nfds; i++) {
                if (fds[i].fd == -1) {
                    for (int j = i; j < nfds - 1; j++) {
                        fds[j].fd = fds[j + 1].fd;
                    }
                    i--;
                    nfds--;
                }
            }
        }
    } while (server_running);

    for (int i = 0; i < nfds; i++) {
        if (fds[i].fd >= 0) {
            close(fds[i].fd);
        }
    }

    return 0;
}

int setup_server(short port, int backlog) {
    int server_socket, client_socket, addr_size;
    SA_IN server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_socket >= 0);

    server_addr = (SA_IN){.sin_family = AF_INET,
                          .sin_addr.s_addr = INADDR_ANY,
                          .sin_port = htons(port)};

    int opt_val = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt_val,
               sizeof opt_val);

    ioctl(server_socket, FIONBIO, &opt_val);

    int err = bind(server_socket, (SA*)&server_addr, sizeof(server_addr));
    assert(err != -1);

    err = listen(server_socket, backlog);
    assert(err != -1);

    return server_socket;
}

int accept_new_conn(int server_socket) {
    int addr_size = sizeof(SA_IN);
    int client_socket;
    SA_IN client_addr;
    client_socket =
        accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size);
    assert(client_socket != -1);

    return client_socket;
}

void* worker(int client_socket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        int read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (!read) {
            break;
        }
        if (read < 0) {
            perror("recv");
        }

        int err = send(client_socket, buffer, read, 0);
        assert(err >= 0);
    }
    close(client_socket);
    return NULL;
}
