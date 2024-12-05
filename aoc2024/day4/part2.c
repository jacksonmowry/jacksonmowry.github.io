#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define oob(x) (x[0] < 0 || x[0] > 139 || x[1] < 0 || x[1] > 139)
#define puzzle_at(x) (puzzle[x[0]][x[1]])

const char *needle = "XMAS";
char puzzle[140][145] = {0};

int explore(int x, int y) {
  int count = 0;

  int deltas[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

  int ul[2] = {x + deltas[0][0], y + deltas[0][1]};
  int ur[2] = {x + deltas[1][0], y + deltas[1][1]};
  int ll[2] = {x + deltas[2][0], y + deltas[2][1]};
  int lr[2] = {x + deltas[3][0], y + deltas[3][1]};

  if (oob(ul) || oob(ur) || oob(ll) || oob(lr)) {
    return 0;
  }

  int mases = 0;

  if ((puzzle_at(ul) == 'M' && puzzle_at(lr) == 'S') ||
      (puzzle_at(ul) == 'S' && puzzle_at(lr) == 'M')) {
    mases++;
  }

  if ((puzzle_at(ur) == 'M' && puzzle_at(ll) == 'S') ||
      (puzzle_at(ur) == 'S' && puzzle_at(ll) == 'M')) {
    mases++;
  }

  return mases == 2;
}

int main() {
  int count = 0;
  int line = 0;
  while (fgets((char *)&puzzle[line++], 145, stdin)) {
  }

  for (int i = 0; i < 140; i++) {
    for (int j = 0; j < 140; j++) {
      if (puzzle[i][j] != 'A') {
        continue;
      }

      count += explore(i, j);
    }
  }

  printf("%d\n", count);
}
