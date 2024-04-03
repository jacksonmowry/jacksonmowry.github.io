#include <asm-generic/socket.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int server_fd, client_fd;

void signal_handler(int signum) {
    if (server_fd != -1) {
        close(server_fd);
    }
    if (client_fd != -1) {
        close(client_fd);
    }

    exit(signum);
}

int main(/* int argc, char** argv */) {
    int err;
    struct sockaddr_in server, client;
    char buf[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd >= 0);

    server = (struct sockaddr_in){.sin_family = AF_INET,
                                  .sin_port = htons(PORT),
                                  .sin_addr.s_addr = htonl(INADDR_ANY)};

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = bind(server_fd, (struct sockaddr*)&server, sizeof(server));
    assert(err >= 0);

    err = listen(server_fd, 128);
    assert(err >= 0);

    printf("Server listening on %d\n", PORT);

    while (true) {
        socklen_t client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr*)&client, &client_len);

        assert(client_fd >= 0);

        while (true) {
            int read = recv(client_fd, buf, BUFFER_SIZE, 0);

            if (!read)
                break; // done
            assert(read >= 0);

            err = send(client_fd, buf, read, 0);
            assert(err >= 0);
        }
    }

    return 0;
}
