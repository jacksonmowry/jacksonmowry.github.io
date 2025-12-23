#include <stddef.h>
#include <stdio.h>
#include <string.h>

int main() {
    size_t shapes[6] = {0};

    char buf[256];
    while (fgets(buf, 256, stdin)) {
        int index;
        sscanf(buf, "%d:", &index);

        while (fgets(buf, 256, stdin)) {
            if (buf[0] == '\n') {
                break;
            }

            buf[strcspn(buf, "\n")] = '\0';
            for (size_t i = 0; buf[i]; i++) {
                shapes[index] += buf[i] == '#';
            }
        }

        if (index == 5) {
            break;
        }
    }

    printf("Shapes: %zu %zu %zu %zu %zu %zu\n", shapes[0], shapes[1], shapes[2],
           shapes[3], shapes[4], shapes[5]);

    size_t fits = 0;
    while (fgets(buf, 256, stdin)) {
        int a;
        int b;
        sscanf(buf, "%dx%d:\n", &a, &b);
        size_t area = a * b;
        size_t running_area = 0;

        char *counts = strstr(buf, ": ") + 2;
        char *ptr = strtok(counts, " \n");
        size_t index = 0;
        do {
            int d;
            sscanf(ptr, "%d", &d);
            running_area += d * shapes[index];
            index++;
        } while ((ptr = strtok(NULL, " \n")));

        if (running_area <= area) {
            fits++;
        }
    }

    printf("Fits: %zu\n", fits);
}
