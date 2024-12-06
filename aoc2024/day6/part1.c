#include <stdio.h>
#include <string.h>

char map[130][135] = {0};
const int dirs[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

int main() {
  int x = -1;
  int y = -1;
  int dir = -1;

  int width;
  int row = 0;
  while (fgets((char *)&map[row++], 134, stdin)) {
    int cursor = 0;
    char current;
    if (x == -1) {
      while ((current = map[row - 1][cursor++])) {
        if (current == '^') {
          x = cursor - 1;
          y = row - 1;
          dir = 0;
        } else if (current == '>') {
          x = cursor - 1;
          y = row - 1;
          dir = 1;
        } else if (current == 'v') {
          x = cursor - 1;
          y = row - 1;
          dir = 2;
        } else if (current == '<') {
          x = cursor - 1;
          y = row - 1;
          dir = 3;
        }
      }
    }
  }

  width = strlen(map[0]);
  int count = 0;
  while (x >= 0 && x < width && y >= 0 && y < row - 1) {
    if (map[y + dirs[dir][0]][x + dirs[dir][1]] == '#') {
      // rotate 90 degrees clockwise
      dir = (dir + 1) % 4;
    } else {
      if (map[y][x] != 'X') {
        count++;
      }
      map[y][x] = 'X';
      x += dirs[dir][1];
      y += dirs[dir][0];
    }
  }

  for (int i = 0; i < row - 1; i++) {
    printf("%s", map[i]);
  }

  printf("%d\n", count);
}
