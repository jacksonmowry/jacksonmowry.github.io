#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static char* reverse(char* str) {
    const int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char tmp = str[len - i - 1];
        str[len - i - 1] = str[i];
        str[i] = tmp;
    }

    return str;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <Port>\n", argv[0]);
        return 1;
    }

    struct addrinfo *rp, *res;
    struct addrinfo hints = {.ai_socktype = SOCK_STREAM,
                             .ai_flags = AI_PASSIVE,
                             .ai_family = AF_INET};
    int errcode;
    int sock;

    // INADDR_ANY
    errcode = getaddrinfo(NULL, argv[1], &hints, &res);
    if (errcode != 0) {
        printf("getaddrinfo: %s\n", gai_strerror(errcode));
        return 1;
    }

    rp = res;
    for (rp = res; rp; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1) {
            continue;
        }

        if (bind(sock, rp->ai_addr, rp->ai_addrlen) != 0) {
            close(sock);
            continue;
        } else {
            break;
        }
    }

    if (!rp) {
        printf("Unable to host server on port %s\n", argv[1]);
        return 1;
    }

    if (listen(sock, 15) != 0) {
        perror("listen");
        return 1;
    }

    int clisock = accept(sock, NULL, NULL);
    if (clisock < 0) {
        perror("accept");
        goto cleanup;
    }

    while (true) {
        char line[256];
        ssize_t bytes;

        bytes = recv(clisock, line, sizeof(line) - 1, 0);
        if (bytes == 0) {
            break;
        }
        line[strcspn(line, "\n")] = '\0';

        bytes = send(clisock, reverse(line), strlen(line), 0);
        if (bytes == 0) {
            break;
        }
    }

    freeaddrinfo(res);

cleanup:
    shutdown(sock, SHUT_RDWR);
    close(sock);
    shutdown(clisock, SHUT_RDWR);
    close(clisock);
}
