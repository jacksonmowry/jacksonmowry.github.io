#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    long unsigned lower[4096] = {0};
    long unsigned upper[4096] = {0};
    long num_ranges = 0;
    long fresh = 0;

    char buf[256];
    while (true) {
        fgets(buf, 256, stdin);
        if (isspace(buf[0])) {
            break;
        }

        sscanf(buf, "%lu-%lu", lower + num_ranges, upper + num_ranges);
        printf("%lu %lu\n", lower[num_ranges], upper[num_ranges]);
        num_ranges++;
    }

    while (fgets(buf, 256, stdin)) {
        long test;
        sscanf(buf, "%ld", &test);

        for (long i = 0; i < num_ranges; i++) {
            if (test >= lower[i] && test <= upper[i]) {
                fresh++;
                break;
            }
        }
    }

    printf("Fresh: %ld\n", fresh);
}
