#include <stdbool.h>
#include <stdio.h>
#include <string.h>

const int deltas[8][2] = {{0, -1},  {1, 0},  {-1, 0}, {0, 1},
                          {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
const char *directions[] = {"left", "down", "up", "right",
                            "UL",   "UR",   "LL", "LR"};
const char *needle = "XMAS";
char puzzle[140][145] = {0};

bool traverse(int x, int y, int dir) {
  int len = 0;

  while (len < 4) {
    if (x < 0 || x > 139 || y < 0 || y > 139) {
      return false;
    }

    if (puzzle[x][y] != needle[len]) {
      return false;
    }

    x += deltas[dir][0];
    y += deltas[dir][1];

    len++;
  }

  return true;
}

int explore(int x, int y) {
  int count = 0;

  for (int i = 0; i < 8; i++) {
    int ret = traverse(x, y, i);
    count += ret;
  }

  return count;
}

int main() {
  int count = 0;
  int line = 0;
  while (fgets((char *)&puzzle[line++], 145, stdin)) {
    /* printf("%s\n", puzzle[line - 1]); */
    /* printf("Reading line %d\n", line); */
  }

  /* for (int i = 0; i < 140; i++) { */
  /*   printf("%s\n", puzzle[i]); */
  /* } */

  for (int i = 0; i < 140; i++) {
    for (int j = 0; j < 140; j++) {
      if (puzzle[i][j] != 'X') {
        continue;
      }

      count += explore(i, j);
    }
  }

  printf("%d\n", count);
}
