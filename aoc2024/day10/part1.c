#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pair {
  int row;
  int col;
} pair_t;

void traverse(int puzzle[256][256], int row, int col, pair_t distinct[256],
              int *distinct_len) {
  if (puzzle[row][col] == 9) {
    distinct[*distinct_len] = (pair_t){.row = row, .col = col};
    *distinct_len = *distinct_len + 1;
    return;
  }

  int cur_val = puzzle[row][col];

  // UP
  if (row != 0 && puzzle[row - 1][col] == cur_val + 1) {
    traverse(puzzle, row - 1, col, distinct, distinct_len);
  }
  // DOWN
  if (puzzle[row + 1][col] != -1 && puzzle[row + 1][col] == cur_val + 1) {
    traverse(puzzle, row + 1, col, distinct, distinct_len);
  }
  // LEFT
  if (col != 0 && puzzle[row][col - 1] == cur_val + 1) {
    traverse(puzzle, row, col - 1, distinct, distinct_len);
  }
  // RIGHT
  if (puzzle[row][col + 1] != -1 && puzzle[row][col + 1] == cur_val + 1) {
    traverse(puzzle, row, col + 1, distinct, distinct_len);
  }
}
int width;

static int cmp(const void *a, const void *b) {
  pair_t p1 = *(pair_t *)a;
  pair_t p2 = *(pair_t *)b;

  int pos1 = (p1.row * width) + p1.col;
  int pos2 = (p2.row * width) + p2.col;

  return pos1 - pos2;
}

int main() {
  int puzzle[256][256];
  memset(puzzle, -1, sizeof(puzzle));
  int row = 0;
  int col = 0;

  char tmp;
  while ((tmp = getc(stdin))) {
    if (tmp == '\n') {
      width = col;
      row++;
      col = 0;

      continue;
    }
    if (tmp == EOF) {
      break;
    }

    puzzle[row][col++] = tmp - '0';
  }

  int total = 0;
  for (int i = 0; i < row; i++) {
    for (int j = 0; puzzle[i][j] != -1; j++) {
      if (puzzle[i][j] != 0) {
        continue;
      }

      pair_t distinct[256];
      int distinct_len = 0;

      traverse(puzzle, i, j, distinct, &distinct_len);

      qsort(distinct, distinct_len, sizeof(pair_t), cmp);

      if (distinct_len > 0) {
        total++;
      }
      for (int i = 1; i < distinct_len; i++) {
        if ((distinct[i].row * width) + distinct[i].col !=
            (distinct[i - 1].row * width) + distinct[i - 1].col) {
          total++;
        }
      }
    }
  }

  printf("%d\n", total);
}
