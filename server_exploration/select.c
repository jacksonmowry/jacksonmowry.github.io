#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
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

int main(int argc, char** argv) {
    printf("%d\n", FD_SETSIZE);
    int server_socket = setup_server(PORT, MAX_CLIENTS);

    fd_set current_sockets, ready_sockets;

    FD_ZERO(&current_sockets);
    FD_SET(server_socket, &current_sockets);

    while (true) {
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == server_socket) {
                    // new conneciton to accept
                    int client_socket = accept_new_conn(server_socket);
                    FD_SET(client_socket, &current_sockets);
                } else {
                    worker(i);
                    FD_CLR(i, &current_sockets);
                }
            }
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
