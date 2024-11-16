#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    struct addrinfo *res, *rp; // keep up with linked list, other is head
    struct addrinfo hints = {.ai_family = AF_UNSPEC,
                             .ai_socktype = SOCK_STREAM,
                             .ai_flags = AI_PASSIVE};
    int sock;

    if (argc != 3) {
        printf("usage: %s <Host> <port>\n", argv[0]);
    }

    int errcode = getaddrinfo(argv[1], argv[2], &hints, &res);
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

        if (connect(sock, rp->ai_addr, rp->ai_addrlen) != 0) {
            close(sock);
        } else {
            break;
        }
    }

    freeaddrinfo(res);

    if (!rp) {
        printf("Unable to make connection to server %s:%s\n", argv[1], argv[2]);
        return 1;
    }

    char line[256];
    ssize_t bytes;
    while (fgets(line, sizeof(line) - 1, stdin) != NULL) {
        bytes = send(sock, line, strlen(line), 0);
        if (bytes == 0) {
            break;
        }

        bytes = recv(sock, line, sizeof(line) - 1, 0);
        if (bytes == 0) {
            break;
        }

        line[bytes] = '\0';
        puts(line);
    }

    close(sock);
}
