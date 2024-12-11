#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char ops[] = {'+', '*', '|'};
uint64_t correct = 0;

bool explore(size_t total, size_t arr_len, int arr[arr_len], size_t pos,
             size_t operators_len, char operators[operators_len]) {
  if (pos == arr_len - 1) {
    size_t accumulator = arr[0];

    for (int i = 1; i < arr_len; i++) {
      switch (operators[i - 1]) {
      case '+':
        accumulator += arr[i];
        break;
      case '*':
        accumulator *= arr[i];
        break;
      case '|': {
        size_t tmp = arr[i];
        while (tmp) {
          accumulator *= 10;
          tmp /= 10;
        }
        accumulator = accumulator + arr[i];
      }

      break;
      }
    }

    if (accumulator == total) {
      return true;
    }

    return false;
  }

  for (int i = 0; i < 3; i++) {
    operators[pos] = ops[i];

    if (explore(total, arr_len, arr, pos + 1, operators_len, operators)) {
      return true;
    }
  }

  return false;
}

int main() {
  char line[256];

  while (fgets(line, sizeof(line), stdin)) {
    int arr[16];
    int arr_len = 0;
    size_t total;
    size_t line_len = strlen(line);

    sscanf(line, "%zu:", &total);

    char *cursor = line;
    while (*cursor != ' ') {
      cursor++;
    }

    char *tmp;
    while ((tmp = strtok(cursor, " "))) {
      arr[arr_len++] = atoi(tmp);
      cursor = NULL;
    }

    char operators[16];
    bool ret = explore(total, arr_len, arr, 0, arr_len, operators);
    correct += ret * total;
  }

  printf("%zu\n", correct);
}
