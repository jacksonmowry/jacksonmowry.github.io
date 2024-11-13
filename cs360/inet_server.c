#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in sin;
    int cli;
    short port;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    if (1 != sscanf(argv[1], "%hd", &port)) {
        printf("Invalid port %s\n", argv[1]);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (const struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return 1;
    }

    listen(sock, 15);
    struct sockaddr_in cli_in;
    socklen_t cli_len = sizeof(cli_in);
    int clisock;

    clisock = accept(sock, (struct sockaddr*)&cli_in, &cli_len);
    if (clisock < 0) {
        perror("accept");
        return 1;
    }

    const char data_out[] = "Hello! Welcome to COSC360!";
    ssize_t actual = send(clisock, data_out, sizeof(data_out), 0);

    if (actual == -1) {
        perror("send");
    }

    char data_int[256];
    actual = recv(clisock, data_int, sizeof(data_int), 0);

    if (actual == 0) {
        close(clisock);
    } else {
        data_int[actual] = '\0';
        printf("I received '%s'\n", data_int);
    }

    if (shutdown(clisock, SHUT_RDWR) < 0) {
        perror("shutdow&");
    }
    close(clisock);

    shutdown(sock, SHUT_RDWR);
    close(sock);
}
