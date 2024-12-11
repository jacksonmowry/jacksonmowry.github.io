#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLAMP(x, max) (x > max ? max : x)

#define ROWS 33
#define COLS 33
#define COLORS 3

typedef struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color_t;

typedef struct Image {
  FILE *fp;
  Color_t *grid;
  size_t grid_height;
  size_t grid_width;
} Image_t;

char grid[(ROWS + 5) / 6][COLORS][COLS];
Color_t colors[COLORS];
size_t colors_len = 0;

int main() {
  printf("\033Pq\n");

  memset(grid, 63, sizeof(grid));
  colors[colors_len++] = (Color_t){.r = 100};
  colors[colors_len++] = (Color_t){.g = 100};
  colors[colors_len++] = (Color_t){.b = 100};

  for (int i = 0; i < ROWS; i++) {
    // Red
    {
      div_t res = div(i, 6);
      assert(res.rem < 6);
      size_t x = i;

      grid[res.quot][0][x] =
          ((grid[res.quot][0][x] - 63) | (int)pow(2, res.rem)) + 63;
    }
    // Green
    {
      div_t res = div(i, 6);
      size_t x = i + 3;

      grid[res.quot][1][x] =
          ((grid[res.quot][1][x] - 63) | (int)pow(2, res.rem)) + 63;
    }
    // Blue
    {
      div_t res = div(i, 6);
      size_t x = 33 - i;

      grid[res.quot][2][x] =
          ((grid[res.quot][2][x] - 63) | (int)pow(2, res.rem)) + 63;
    }
  }

  for (int i = 0; i < COLORS; i++) {
    printf("#%d;2;%hu;%hu;%hu\n", i, colors[i].r, colors[i].g, colors[i].b);
  }

  for (int i = 0; i < ((ROWS + 5) / 6); i++) {
    for (int j = 0; j < COLORS; j++) {
      printf("#%d%.*s$\n", j, COLS, grid[i][j]);
    }
    puts("-");
  }

  puts("#0;2;0;0;0");
  puts("#1;2;0;0;0");
  puts("#2;2;0;0;0");

  puts("");

  printf("\033\\\n");
}
