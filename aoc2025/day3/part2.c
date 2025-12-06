#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char buf[256];
    long total = 0;

    while (fgets(buf, sizeof(buf), stdin)) {
        size_t len = strlen(buf);
        buf[strcspn(buf, "\n")] = '\0';
        len -= 1;

        int numbers[12] = {0};
        int pos[12] = {0};

        for (size_t i = 0; i < 12; i++) {
            // One loop per battery
            // We need to stop 12 - i - 1 batteries short
            int start = 0;
            if (i != 0) {
                start = pos[i - 1] + 1;
            }
            for (size_t j = start; j < len - (12 - i - 1); j++) {
                if (buf[j] > numbers[i]) {
                    numbers[i] = buf[j];
                    pos[i] = j;
                }
            }
        }

        char final_buf[13] = {
            numbers[0], numbers[1], numbers[2],  numbers[3],
            numbers[4], numbers[5], numbers[6],  numbers[7],
            numbers[8], numbers[9], numbers[10], numbers[11],
        };

        long long num = atoll(final_buf);

        total += num;
    }

    printf("Total: %ld\n", total);
}
