#include <asm-generic/socket.h>
#include <assert.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080
#define MAX_CLIENTS 1024

int server_fd, client_fd;

void* worker(void* args) {
    int client_socket = *(int*)args;
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

int main(/* int argc, char** argv */) {
    int err;
    struct sockaddr_in server, client;
    pthread_t thread_id;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd >= 0);

    server = (struct sockaddr_in){.sin_family = AF_INET,
                                  .sin_port = htons(PORT),
                                  .sin_addr.s_addr = htonl(INADDR_ANY)};

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = bind(server_fd, (struct sockaddr*)&server, sizeof(server));
    assert(err >= 0);

    err = listen(server_fd, MAX_CLIENTS);
    assert(err >= 0);

    printf("Server listening on %d\n", PORT);

    while (true) {
        socklen_t client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr*)&client, &client_len);

        assert(client_fd >= 0);

        int err_code =
            pthread_create(&thread_id, NULL, worker, (void*)&client_fd);
        assert(err_code == 0);

        pthread_detach(thread_id);
    }

    return 0;
}
