#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    struct addrinfo hints = {.ai_family = AF_UNSPEC,
                             .ai_socktype = SOCK_STREAM,
                             .ai_flags = AI_PASSIVE};
    struct addrinfo *r, *rp;
    int s;
    int sock;

    if (argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        return 1;
    }

    s = getaddrinfo(argv[1], argv[2], &hints, &r);
    if (s != 0) {
        printf("getaddrinfo: %s\n", gai_strerror(s));
        return 1;
    }

    for (rp = r; rp != NULL; rp = rp->ai_next) {
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

    freeaddrinfo(r);

    if (rp == NULL) {
        printf("could not connect to %s:%s\n", argv[1], argv[2]);
        return 1;
    }

    char line[256];
    ssize_t bytes;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        bytes = send(sock, line, strlen(line), 0);

        bytes = recv(sock, line, sizeof(line) - 1, 0);
        if (bytes == 0) {
            break;
        }
        line[bytes] = '\0';
        printf("%s\n", line);
    }

    shutdown(sock, SHUT_RDWR);
    close(sock);
}
