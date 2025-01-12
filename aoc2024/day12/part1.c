#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct Pair {
  uint64_t area;
  uint64_t perimiter;
} Pair_t;

char board[256][256];
bool seen[256][256];

size_t rows;
size_t cols;

void recurse(Pair_t *p, char plot_type, int row, int col) {
  if (board[row][col] != plot_type) {
    return;
  }

  if (seen[row][col]) {
    return;
  }

  seen[row][col] = true;
  p->area++;

  // UP
  if (row != 0 && board[row - 1][col] == plot_type) {
    recurse(p, plot_type, row - 1, col);
  } else {
    p->perimiter++;
  }
  // DOWN
  if (row < rows - 1 && board[row + 1][col] == plot_type) {
    recurse(p, plot_type, row + 1, col);
  } else {
    p->perimiter++;
  }
  // LEFT
  if (col != 0 && board[row][col - 1] == plot_type) {
    recurse(p, plot_type, row, col - 1);
  } else {
    p->perimiter++;
  }
  // RIGHT
  if (col < cols - 1 && board[row][col + 1] == plot_type) {
    recurse(p, plot_type, row, col + 1);
  } else {
    p->perimiter++;
  }

  return;
}

int main() {
  while (fgets(board[rows++], 256, stdin)) {
    board[rows - 1][strcspn(board[rows - 1], "\n")] = 0;
    printf("read row %zu\n", rows);
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
      recurse(&result, board[i][j], i, j);

      printf("%c | area: %zu perimiter: %zu\n", board[i][j], result.area,
             result.perimiter);

      total += result.area * result.perimiter;
    }
  }

  printf("%zu\n", total);
}
