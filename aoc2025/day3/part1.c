#include <stdio.h>
#include <string.h>

int main() {
    char buf[256];
    long total = 0;

    while (fgets(buf, sizeof(buf), stdin)) {
        size_t len = strlen(buf);
        int max = 0;
        int max_pos = -1;

        for (size_t i = 0; i < len - 2; i++) {
            if (buf[i] > max) {
                max = buf[i];
                max_pos = i;
            }
        }

        int second_max = 0;
        for (size_t i = max_pos + 1; i < len - 1; i++) {
            if (buf[i] > second_max) {
                second_max = buf[i];
            }
        }

        printf("Picked: %d & %d\n", max - '0', second_max - '0');
        total += (max - '0') * 10 + (second_max - '0');
    }

    printf("Total: %ld\n", total);
}
