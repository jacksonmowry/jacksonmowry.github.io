#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Pair {
  int num;
  int rank;
} Pair_t;

bool before[100][100] = {0};

int my_comp(const void *a, const void *b);

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
    int input_parallel[25] = {0};
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
        } else {
          /* input_parallel[i]++; */
        }
      }
    }

    if (valid) {
      continue; // Skip this one, it's valid
    }

    for (int i = 0; i < input_len; i++) {
      for (int j = 0; j < input_len; j++) {
        if (before[input[i]][input[j]]) {
          input_parallel[i]++;
        }
      }
    }

    Pair_t arr[input_len];
    for (int i = 0; i < input_len; i++) {
      arr[i] = (Pair_t){.num = input[i], .rank = input_parallel[i]};
    }

    qsort(arr, input_len, sizeof(Pair_t), my_comp);

    total += arr[input_len / 2].num;

    for (int i = 0; i < input_len; i++) {
      printf("%d ", arr[i].num);
    }
    printf("| ");
    for (int i = 0; i < input_len; i++) {
      printf("%d ", arr[i].rank);
    }
    putchar('\n');
  }

  printf("%d\n", total);
}

int my_comp(const void *a, const void *b) {
  Pair_t left = *(Pair_t *)a;
  Pair_t right = *(Pair_t *)b;

  if (left.rank > right.rank) {
    return -1;
  } else if (left.rank < right.rank) {
    return 1;
  } else {
    // Tie in rank, determine which must come first
    if (before[left.num][right.num]) {
      return -1;
    } else {
      return 1;
    }
    return 0;
  }
}
