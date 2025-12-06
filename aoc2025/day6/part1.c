#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    unsigned long long addition[4096] = {0};
    unsigned long long multiplication[4096] = {0};
    char digits[4][4096] = {0};
    for (size_t i = 0; i < 4096; i++) {
        multiplication[i] = 1;
    }

    char buf[4096] = {0};
    while (fgets(buf, 4096, stdin)) {
        if (buf[0] == '+' || buf[0] == '*') {
            // Handle finalization logic
            unsigned long long sum = 0;

            char *ptr = strtok(buf, " ");
            size_t column = 0;
            do {
                if (ptr[0] == '+') {
                    sum += addition[column];
                } else if (ptr[0] == '*') {
                    sum += multiplication[column];
                }

                column++;
            } while ((ptr = strtok(NULL, " ")));

            printf("Final sum: %llu\n", sum);
        }

        size_t column = 0;
        char *ptr = strtok(buf, " ");
        do {
            unsigned long long x = strtoull(ptr, NULL, 0);
            addition[column] += x;
            multiplication[column] *= x;
            column++;
        } while ((ptr = strtok(NULL, " ")));
    }
}
