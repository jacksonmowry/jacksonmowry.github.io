#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct Pair {
  uint64_t area;
  uint64_t sides;
} Pair_t;

char board[256][256];
bool seen[256][256];

size_t rows;
size_t cols;

void recurse(Pair_t *p, char plot_type, int row, int col, int sides_mask) {
  if (board[row][col] != plot_type) {
    return;
  }

  if (seen[row][col]) {
    return;
  }

  seen[row][col] = true;
  p->area++;

  int sides = 0;

  // og loops
  // UP
  if (row != 0 && board[row - 1][col] == plot_type) {
    recurse(p, plot_type, row - 1, col, sides);
  } else {
    sides |= 1 << 3;
  }
  // DOWN
  if (row < rows - 1 && board[row + 1][col] == plot_type) {
    recurse(p, plot_type, row + 1, col, sides);
  } else {
    sides |= 1 << 2;
  }
  // LEFT
  if (col != 0 && board[row][col - 1] == plot_type) {
    recurse(p, plot_type, row, col - 1, sides);
  } else {
    sides |= 1 << 1;
  }
  // RIGHT
  if (col < cols - 1 && board[row][col + 1] == plot_type) {
    recurse(p, plot_type, row, col + 1, sides);
  } else {
    sides |= 1 << 0;
  }

  if ((sides & 0b1010) == 0b1010) {
    p->sides++;

    if (sides == 0b1010 && (board[row + 1][col + 1] != plot_type ||
                            row + 1 >= rows || col + 1 >= cols)) {
      p->sides++;
    }
  }
  if ((sides & 0b1001) == 0b1001) {
    p->sides++;

    if (sides == 0b1001 &&
        (board[row + 1][col - 1] != plot_type || row + 1 >= rows || col == 0)) {
      p->sides++;
    }
  }
  if ((sides & 0b0110) == 0b0110) {
    p->sides++;

    if (sides == 0b0110 &&
        (board[row - 1][col + 1] != plot_type || row == 0 || col + 1 >= cols)) {
      p->sides++;
    }
  }
  if ((sides & 0b0101) == 0b0101) {
    p->sides++;

    if (sides == 0b0101 &&
        (board[row - 1][col - 1] != plot_type || row == 0 || col == 0)) {
      p->sides++;
    }
  }

  // side upd
  if (sides & 0b1000) {
  }

  return;
}

int main() {
  while (fgets(board[rows++], 256, stdin)) {
    board[rows - 1][strcspn(board[rows - 1], "\n")] = 0;
  }

  rows--;
  cols = strlen(board[0]);

  for (int i = 0; i < rows; i++) {
    printf("%s\n", board[i]);
  }

  uint64_t total = 0;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (seen[i][j]) {
        continue;
      }

      Pair_t result = {};
      recurse(&result, board[i][j], i, j, 0);

      printf("%c | area: %zu sides: %zu\n", board[i][j], result.area,
             result.sides);

      total += result.area * result.sides;
    }
  }

  printf("%zu\n", total);
}
