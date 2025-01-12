#pragma once

#include "color.h"
#include <stdbool.h>

typedef struct Image {
  PackedColor_t *grid;
  bool *used;
  size_t grid_height;
  size_t grid_width;
} Image_t;

void image_add_pixel(Image_t *i, Color_t c, size_t row, size_t col);

void image_draw_rect(Image_t *i, Color_t c, int x1, int y1, int x2, int y2);

void color_register_setup(FILE *fp);

void color_register_teardown(FILE *fp);

void print_sixel(FILE *fp, Image_t *img);
