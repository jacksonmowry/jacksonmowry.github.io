#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main() {
    char map[164][164] = {0};
    memset(map, '_', sizeof(map));
    size_t rows = 0;
    size_t len;
    size_t cols;

    while (fgets(map[rows++ + 1] + 1, 163, stdin)) {
        map[rows][strcspn(map[rows], "\n")] = '_';
    }
    cols = strcspn(map[1], "\n") - 2;

    for (size_t i = 0; i < 164; i++) {
        map[i][163] = '\0';
    }

    for (size_t i = 0; i < rows + 1; i++) {
        printf("%s\n", map[i]);
    }

    bool remove[164][164] = {0};

    size_t total_count = 0;
    size_t round_count = 1;

    while (round_count != 0) {
        round_count = 0;

        for (size_t row = 1; row <= rows; row++) {
            for (size_t col = 1; col <= cols; col++) {
                if (map[row][col] != '@') {
                    continue;
                }

                int adjacent = 0;

                adjacent += map[row - 1][col] == '@';
                adjacent += map[row + 1][col] == '@';
                adjacent += map[row][col - 1] == '@';
                adjacent += map[row][col + 1] == '@';
                adjacent += map[row - 1][col - 1] == '@';
                adjacent += map[row - 1][col + 1] == '@';
                adjacent += map[row + 1][col - 1] == '@';
                adjacent += map[row + 1][col + 1] == '@';

                if (adjacent < 4) {
                    round_count++;
                    remove[row][col] = true;
                }
            }
        }

        total_count += round_count;

        for (int i = 0; i < 164; i++) {
            for (int j = 0; j < 164; j++) {
                if (remove[i][j]) {
                    map[i][j] = '.';
                }
            }
        }
    }

    printf("Count: %zu\n", total_count);
    printf("rows %zu\n", rows);
    printf("cols %zu\n", cols);
}
