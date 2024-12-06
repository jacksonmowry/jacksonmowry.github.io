#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool before[100][100] = {0};

int main() {
  char line[256];

  // phase 1
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = '\0';

    if (strlen(line) == 0) {
      // goto phase 2
      break;
    }

    int a, b;
    sscanf(line, "%d|%d", &a, &b);

    before[a][b] = true;
  }

  // phase 2
  int total = 0;
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = '\0';
    int input[25];
    int input_len = 0;

    char *cursor = line;

    while (*cursor) {
      sscanf(cursor, "%d", input + input_len++);
      while (*cursor != ',' && *cursor != '\0') {
        cursor++;
      }
      if (*cursor != '\0') {
        cursor++;
      }
    }

    bool valid = true;
    for (int i = 0; i < input_len; i++) {
      for (int j = i + 1; j < input_len; j++) {
        if (!before[input[i]][input[j]]) {
          valid = false;
          break;
        }
      }
    }

    if (valid) {
      total += input[input_len / 2];
    }
  }

  printf("%d\n", total);
}
