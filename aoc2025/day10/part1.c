#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct Thing {
    uint16_t target;
    uint16_t ops[16];
    size_t num_ops;
} Thing;

int main() {
    Thing things[1024] = {0};
    size_t num_things = 0;

    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin)) {
        char *ptr = strtok(buf, " ");

        do {
            if (ptr[0] == '[') {
                printf("ptr is \"%s\", in '[' loop\n", ptr);
                ptr++;
                int where = 15;
                while (ptr[0] && ptr[0] != ']') {
                    things[num_things].target |= (ptr[0] == '#') << (where--);
                    ptr++;
                };
            } else if (ptr[0] == '(') {
                ptr++;
                while (ptr[0] && ptr[0] != ')') {
                    if (ptr[0] == ',') {
                        ptr++;
                        continue;
                    }

                    // At a number
                    things[num_things].ops[things[num_things].num_ops] |=
                        1 << (15 - (ptr[0] - '0'));
                    ptr++;
                }

                // done
                things[num_things].num_ops++;
            } else if (ptr[0] == '{') {
                num_things++;
                break;
            }
        } while ((ptr = strtok(NULL, " ")));
    }

    printf("We have %zu things\n", num_things);
    for (size_t i = 0; i < num_things; i++) {
        printf("%8hu ", things[i].target);
        printf("[");
        for (size_t j = 0; j < 16; j++) {
            printf("%c", (things[i].target >> (16 - j - 1) & 1) ? '#' : '.');
        }
        printf("]");
        for (size_t j = 0; j < things[i].num_ops; j++) {
            printf("%hu ", things[i].ops[j]);
        }
        printf("\n");
    }

    size_t counter = 0;
    for (size_t i = 0; i < num_things; i++) {
        printf("Solving for %hu\n", things[i].target);
        uint32_t enumerator = 0;
        uint32_t upper_bound = 1 << things[i].num_ops;
        int min = 64;

        while (enumerator < upper_bound) {
            uint16_t t = 0;
            for (size_t j = 0; j < 16; j++) {
                if ((enumerator >> j) & 1) {
                    t ^= things[i].ops[j];
                }
            }

            if (t == things[i].target) {
                int count = __builtin_popcount(enumerator);
                printf("\tCount of %d, min of %d\n", count, min);
                if (count < min) {
                    printf("\t\tUpdating min\n");
                    min = count;
                }
            }

            enumerator++;
        }
        counter += min;
    }

    printf("Answer: %zu\n", counter);
}
