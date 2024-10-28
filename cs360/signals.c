#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

bool loopit = true;

static void sighandle(int sig) {
    if (sig == SIGINT) {
        loopit = false;
    } else if (sig == SIGSEGV) {
        printf("Segmentation fault :)\n");
    }
}

int main() {
    signal(SIGINT, sighandle);
    signal(SIGSEGV, sighandle);

    int* i = 0;

    *i = 12345;

    printf("Ok I'll stop %d\n", *i);
    return 0;
}
