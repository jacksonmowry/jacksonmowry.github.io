#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CLAMP(x, max) (x > max ? max : x)

#define ROWS 33
#define COLS 33
#define COLORS 3

typedef struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color_t;

char *as_sixel(Color_t color, int number) {
  char *ret = NULL;
  asprintf(&ret, "%d;2;%hu;%hu;%hu", number, color.r, color.g, color.b);
  return ret;
}

typedef struct Image {
  FILE *fp;
  Color_t *grid;
  size_t grid_height;
  size_t grid_width;
} Image_t;

uint8_t grid[ROWS][COLORS][COLS];
Color_t colors[COLORS];
size_t colors_len = 0;

int main() {
  for (int i = 0; i < ROWS; i++) {
    // Red
    { div_t res = div(i, 6); }
  }
}
