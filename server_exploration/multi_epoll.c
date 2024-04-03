#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080
#define MAX_CLIENTS 1024
#define THREAD_COUNT 12

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
    bzero((char*)addr, sizeof(struct sockaddr_in));
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

typedef struct worker_args {
    struct epoll_event events[MAX_CLIENTS];
    int epfd, nfds;
} worker_args;

void* worker(void* arg) {
    worker_args* wa = (worker_args*)arg;
    char buffer[BUFFER_SIZE];
    wa->epfd = epoll_create(1);
    while (true) {
        wa->nfds = epoll_wait(wa->epfd, wa->events, MAX_CLIENTS, -1);
        for (int i = 0; i < wa->nfds; i++) {
            if (wa->events[i].events & EPOLLIN) {
                int incoming_socket = wa->events[i].data.fd;
                int read = recv(incoming_socket, buffer, BUFFER_SIZE, 0);
                if (read <= 0) {
                    epoll_ctl(wa->epfd, EPOLL_CTL_DEL, incoming_socket, NULL);
                    shutdown(incoming_socket, SHUT_RDWR);
                } else {
                    send(incoming_socket, buffer, read, 0);
                }
            }
        }
    }
    return NULL;
}

int main() {
    pthread_t threads[THREAD_COUNT];
    worker_args thread_args[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, worker, &thread_args[i]);
    }

    int server_socket, client_socket;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    int epfd, nfds;
    struct epoll_event events[MAX_CLIENTS];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    set_sockaddr(&server_addr);
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    set_non_blocking(server_socket);
    listen(server_socket, MAX_CLIENTS);

    epfd = epoll_create(1);
    epoll_ctl_add(epfd, server_socket, EPOLLIN | EPOLLOUT | EPOLLET);

    uint32_t socklen = sizeof(client_addr);

    while (true) {
        nfds = epoll_wait(epfd, events, MAX_CLIENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_socket) {
                // new incoming connection
                client_socket = accept(
                    server_socket, (struct sockaddr*)&client_addr, &socklen);

                set_non_blocking(client_socket);
                int thread = rand() % 12;
                epoll_ctl_add(thread_args[thread].epfd, client_socket,
                              EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);
            }
        }
    }
}
