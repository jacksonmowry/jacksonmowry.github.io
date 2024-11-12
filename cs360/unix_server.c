#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define FILENAME "myunix.sock"

int main() {
    int sock;
    struct sockaddr_un sun;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("socket");
        return (1);
    }

    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, FILENAME, sizeof(sun.sun_path) - 1);

    if (bind(sock, (const struct sockaddr*)&sun, sizeof(sun)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(sock, 10) < 0) {
        perror("listen");
        return 1;
    }

    struct sockaddr_un client_sun;
    socklen_t client_socklen = sizeof(client_sun);
    int clients_fd;
    fprintf(stderr, "Before connect\n");
    if ((clients_fd = accept(sock, (struct sockaddr*)&client_sun,
                             &client_socklen)) < 0) {
    }
    fprintf(stderr, "After connect\n");

    close(sock);
    unlink(FILENAME);
}
