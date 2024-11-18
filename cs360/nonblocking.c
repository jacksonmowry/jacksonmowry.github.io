#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void setblocking(int fd, bool block) {
    long flags = fcntl(fd, F_GETFL);

    if (block) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }

    fcntl(fd, F_SETFL, flags);
}

int main() {
    int fd = STDIN_FILENO;
    char buffer[256];
    ssize_t bytes;

    setblocking(fd, false);
    for (;;) {
        bytes = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes <= 0 && errno != EAGAIN) {
            return 1;
        }

        buffer[bytes] = 0;
        buffer[strcspn(buffer, "\n")] = 0;
        printf("%s\n", buffer);
    }

    close(fd);
}
