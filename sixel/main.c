#include "color.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CLAMP(x, max) (x > max ? max : x)
bool used[256];

typedef struct Image {
  PackedColor_t *grid;
  size_t grid_height;
  size_t grid_width;
} Image_t;

void image_add_pixel(Image_t *i, Color_t c, size_t row, size_t col) {
  PackedColor_t pc = packed_from_color(c);
  used[packed_index(pc)] = true;

  i->grid[(row * i->grid_width) + col] = pc;
}

void image_draw_rect(Image_t *i, Color_t c, int x1, int y1, int x2, int y2) {
  if (x1 < 0 || x1 >= i->grid_width || x2 < 0 || x2 >= i->grid_width) {
    fprintf(stderr, "oob x\n");
    return;
  }
  if (y1 < 0 || y1 >= i->grid_height || y2 < 0 || y2 >= i->grid_height) {
    fprintf(stderr, "oob y\n");
    return;
  }

  for (int row = y1; row <= y2; row++) {
    for (int col = x1; col <= x2; col++) {
      image_add_pixel(i, c, row, col);
    }
  }
}

void color_register_setup() {
  for (uint16_t i = 0; i < 256; i++) {
    PackedColor_t pc = (PackedColor_t){.r = (i & 0b11100000) >> 5,
                                       .g = (i & 0b00011100) >> 2,
                                       .b = (i & 0b00000011)};
    int r_intensity = pc.r * (1 / 0.08);
    int g_intensity = pc.g * (1 / 0.08);
    int b_intensity = pc.b * (1 / 0.04);

    printf("#%d;2;%hu;%hu;%hu\n", i, r_intensity, g_intensity, b_intensity);
  }
}

void color_register_teardown() {
  for (uint16_t i = 0; i < 256; i++) {
    printf("#%d;2;%hu;%hu;%hu\n", i, 0, 0, 0);
  }
}

void print_sixel(Image_t *img) {
  for (int row = 0; row < img->grid_height; row += 6) {
    for (int color = 0; color < 256; color++) {
      if (used[color] != true) {
        continue;
      }
      printf("#%d", color);
      for (int col = 0; col < img->grid_width; col++) {
        char byte = 0;
        for (int six = 0; six < 6 && (row + six) < img->grid_height; six++) {
          if (packed_index(img->grid[(row + six) * img->grid_width + col]) ==
              color) {
            byte |= 1 << six;
          }
        }
        printf("%c", byte + 63);
      }
      printf("$\n");
    }
    printf("-\n");
  }
}

#define WIDTH 256
#define HEIGHT 256
int main() {

  for (int frame = 0; frame < 150; frame++) {
    printf("\033[H");
    puts("");

    printf("\033Pq\n");
    color_register_setup();

    Image_t i = (Image_t){.grid = malloc(WIDTH * HEIGHT * sizeof(*i.grid)),
                          .grid_height = HEIGHT,
                          .grid_width = WIDTH};

    image_draw_rect(&i, color_from_intensity(0.75, 0.75, 0.75), 1, 1, 250, 250);
    image_draw_rect(&i, color_from_intensity(0.5, 0.5, 0.5), 150 - frame,
                    150 - frame, 250 - frame, 250 - frame);

    print_sixel(&i);

    color_register_teardown();

    puts("");

    printf("\033\\\n\n");
    /* usleep(50000); */
    printf("\033[H");
  }
}
