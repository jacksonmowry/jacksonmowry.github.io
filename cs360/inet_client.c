#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    struct sockaddr_in sin;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(9999);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (const struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("connect");
        return 1;
    }

    char data[256];
    int data_len;

    data_len = recv(sock, data, sizeof(data), 0);
    data[data_len] = '\0';
    printf("%s\n", data);

    send(sock, "Yay!", strlen("Yay!"), 0);

    shutdown(sock, SHUT_RDWR);
    close(sock);
}
