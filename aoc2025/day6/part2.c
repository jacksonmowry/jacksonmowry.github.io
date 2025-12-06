#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Blah {
    int start_col;
    int stop_col;
    char op;
} Blah;

int main() {
    char digits[4][4096][4] = {0};
    Blah problems[4096] = {0};
    size_t num_problems = 0;

    char buf[4096] = {0};
    size_t row = 0;
    while (fgets(buf, 4096, stdin)) {
        if (buf[0] == '+' || buf[0] == '*') {
            // Record starting and stopping column for each problem
            problems[0].start_col = 0;
            problems[0].op = buf[0];
            size_t buf_len = strlen(buf);
            for (size_t column = 1; column < buf_len; column++) {
                if (buf[column] == '+' || buf[column] == '*') {
                    problems[num_problems].stop_col = column - 1;

                    num_problems++;
                    problems[num_problems].start_col = column;
                    problems[num_problems].op = buf[column];
                } else if (buf[column] == '\n') {
                    problems[num_problems].stop_col = column - 1;
                    num_problems++;
                }
            }

        } else {
            for (size_t i = 0; i < num_problems; i++) {
                for (size_t j = problems[i].start_col;
                     j <= problems[i].stop_col; j++) {
                    digits[row - 1][i][j - problems[i].start_col] = buf[j];
                }
            }
        }

        row++;
    }

    // Handle finalization logic
    unsigned long long sum = 0;

    for (size_t column = 0; column < num_problems; column++) {
        unsigned long long numbers[4] = {0};
        for (size_t i = 0; i < row - 1; i++) {
            for (size_t val = 0; val < 4 && digits[i][column][val]; val++) {
                if (digits[i][column][val] == ' ') {
                    continue;
                }
                numbers[val] *= 10;
                numbers[val] += digits[i][column][val] - '0';
            }
        }
        printf("%llu %llu %llu %llu\n", numbers[0], numbers[1], numbers[2],
               numbers[3]);

        if (problems[column].op == '+') {
            sum += numbers[0] + numbers[1] + numbers[2] + numbers[3];
        } else if (problems[column].op == '*') {
            sum += numbers[0] * numbers[1] *
                   ((numbers[2] == 0) ? 1 : numbers[2]) *
                   ((numbers[3] == 0) ? 1 : numbers[3]);
        }
    }

    printf("Final sum: %llu\n", sum);
}
