#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8069
#define MAX_CLIENTS 8192

static void epoll_ctl_add(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl()\n");
        exit(1);
    }
}

static void set_sockaddr(struct sockaddr_in* addr) {
    memset((char*)addr, 0, sizeof(struct sockaddr_in));
    *addr = (struct sockaddr_in){.sin_family = AF_INET,
                                 .sin_addr.s_addr = INADDR_ANY,
                                 .sin_port = htons(PORT)};
}

static int set_non_blocking(int sockfd) {
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) == -1) {
        return -1;
    }

    return 0;
}

int server_socket;
void signal_handler(int num) {
    int code = close(server_socket);
    if (code != 0) {
        perror("close");
    }
    exit(0);
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    int client_socket;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    int epfd, nfds;
    struct epoll_event events[MAX_CLIENTS];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    set_sockaddr(&server_addr);
    int code = bind(server_socket, (struct sockaddr*)&server_addr,
                    sizeof(server_addr));
    if (code == -1) {
        perror("bind");
        exit(1);
    }

    set_non_blocking(server_socket);
    code = listen(server_socket, MAX_CLIENTS);
    if (code == -1) {
        perror("listen");
        exit(1);
    }

    epfd = epoll_create(1);
    epoll_ctl_add(epfd, server_socket, EPOLLIN | EPOLLOUT | EPOLLET);

    uint32_t socklen = sizeof(client_addr);

    while (true) {
        nfds = epoll_wait(epfd, events, MAX_CLIENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_socket) {
                // This is a new incoming connection
                client_socket = accept(
                    server_socket, (struct sockaddr*)&client_addr, &socklen);
                set_non_blocking(client_socket);
                epoll_ctl_add(epfd, client_socket,
                              EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);
            } else if (events[i].events & EPOLLIN) {
                // Handling incoming request
                int incoming_socket = events[i].data.fd;
                int read = recv(incoming_socket, buffer, BUFFER_SIZE, 0);
                if (read <= 0) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, incoming_socket, NULL);
                    close(incoming_socket);
                } else {
                    // Open and send requested file
                    char* method = strtok(buffer, " ");
                    char* path = strtok(NULL, " ");
                    path++;
                    char* filepath = realpath(path, NULL);

                    // Stat for filesize
                    struct stat st;
                    stat(filepath, &st);

                    // Send HTTP Header
                    size_t header_length = snprintf(
                        buffer, sizeof(buffer) - 1,
                        "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: "
                        "text/plain\r\nContent-Length: %ld\r\n\r\n",
                        st.st_size);
                    int code = send(incoming_socket, buffer, header_length, 0);
                    if (code < 0) {
                        free(filepath);

                        epoll_ctl(epfd, EPOLL_CTL_DEL, incoming_socket, NULL);
                        code = close(incoming_socket);
                        continue;
                    }

                    // Open file for sendfile
                    int fd = open(filepath, O_RDONLY);
                    if (fd == -1) {
                        perror("open");
                        exit(1);
                    }
                    free(filepath);

                    // Send file contents to client
                    code = sendfile(incoming_socket, fd, NULL, st.st_size);
                    if (code < 0) {
                        code = close(incoming_socket);
                        code = close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, incoming_socket, NULL);
                        continue;
                    }

                    // Close open fd
                    code = close(fd);
                    if (code == -1) {
                        perror("close");
                        exit(1);
                    }
                    // TEMPORARY SHUTDOWN
                    code = close(incoming_socket);
                    if (code == -1) {
                        perror("close");
                        exit(1);
                    }
                    epoll_ctl(epfd, EPOLL_CTL_DEL, incoming_socket, NULL);
                    // TEMPORARY SHUTDOWN
                }
            }
        }
    }
}
