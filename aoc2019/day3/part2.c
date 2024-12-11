#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Pair {
  int x;
  int y;
} Pair_t;

Pair_t first[1000000] = {};
Pair_t second[1000000] = {};
size_t first_len = 0;
size_t second_len = 0;
size_t intersections_len = 0;

int main() {
  int dist;
  char dir;

  // First
  Pair_t prev = {0, 0};
  while (scanf("%c%d", &dir, &dist) == 2) {
    while (dist) {
      switch (dir) {
      case 'L':
        prev.x -= 1;
        break;
      case 'R':
        prev.x += 1;
        break;
      case 'U':
        prev.y += 1;
        break;
      case 'D':
        prev.y -= 1;
        break;
      }

      dist--;
      first[first_len++] = prev;
    }

    if (getc(stdin) == '\n') {
      break;
    }
  }
  // Second
  prev = (Pair_t){0, 0};
  while (scanf("%c%d", &dir, &dist) == 2) {
    while (dist) {
      switch (dir) {
      case 'L':
        prev.x -= 1;
        break;
      case 'R':
        prev.x += 1;
        break;
      case 'U':
        prev.y += 1;
        break;
      case 'D':
        prev.y -= 1;
        break;
      }

      dist--;
      second[second_len++] = prev;
    }
    if (getc(stdin) == EOF) {
      break;
    }
  }

  /* for (int i = 0; i < first_len; i++) { */
  /*   printf("(%d,%d)\n", first[i].x, first[i].y); */
  /* } */

  uint64_t shortest = UINT64_MAX;

  for (int i = 0; i < first_len; i++) {
    for (int j = 0; j < second_len; j++) {
      if (first[i].x == second[j].x && first[i].y == second[j].y &&
          (abs(first[i].x) + abs(first[i].y)) < shortest) {
        shortest = abs(first[i].x) + abs(first[i].y);
      }
    }
  }

  printf("%zu\n", shortest);
}
