#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    char buffer[256];
    ssize_t bytes;

    fd_set rds;
    struct timeval tv;

    size_t iterations = 0;

    for (;;) {
        FD_ZERO(&rds);
        FD_SET(STDIN_FILENO, &rds);

        int ret = select(1, &rds, NULL, NULL,
                         &(struct timeval){.tv_sec = 0, .tv_usec = 0});
        if (FD_ISSET(STDIN_FILENO, &rds)) {
            // Readable data!
            puts("Data is available");
            bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (bytes <= 0) {
                break;
            }
            buffer[bytes] = '\0';
            buffer[strcspn(buffer, "\n")] = '\0';
            printf("data = '%s'\n", buffer);
        } else {
            iterations += 1;
        }
    }

    printf("iterations = %zu\n", iterations);
}
