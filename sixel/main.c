#include "color.h"
#include "image.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CLAMP(x, max) (x > max ? max : x)
bool used[256];

#define WIDTH 256
#define HEIGHT 256
int main() {
  printf("\033[2J");
  fflush(stdout);

  size_t frame = 0;

  float x1 = 10;
  float y1 = 30;
  float x2 = 20;
  float y2 = 40;

  float v_x = 1;
  float v_y = 1;

  while (true) {
    printf("\033[H\n");

    printf("\033Pq\n");
    color_register_setup(stdout);

    Image_t i = (Image_t){.grid = malloc(WIDTH * HEIGHT * sizeof(*i.grid)),
                          .used = malloc(256 * sizeof(bool)),
                          .grid_height = HEIGHT,
                          .grid_width = WIDTH};

    image_draw_rect(&i, color_from_intensity(0.75, 0.75, 0.75), 0, 0, 255, 255);
    image_draw_rect(&i, color_from_intensity(0.5, 0.5, 0.5), x1, y1, x2, y2);

    if (x1 + v_x < 0 || x2 + v_x >= i.grid_width) {
      v_x *= -(rand() / (float)RAND_MAX);
    }
    if (y1 + v_y < 0 || y2 + v_y >= i.grid_height) {
      v_y *= -(rand() / (float)RAND_MAX);
    }

    x1 += v_x;
    x2 += v_x;
    y1 += v_y;
    y2 += v_y;

    print_sixel(stdout, &i);

    color_register_teardown(stdout);

    puts("");

    printf("\033\\\n\n");
    /* usleep(5000); */
    printf("\033[H");
  }
}
