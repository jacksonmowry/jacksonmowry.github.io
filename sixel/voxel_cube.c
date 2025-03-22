#include "color.h"
#include "image.h"
#include "unistd.h"
#include <assert.h>
#include <fcntl.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HEIGHT 45
#define WIDTH 60

int main(int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "rb");

  Image_t img = (Image_t){.grid = malloc(8 * 60 * 8 * 45 * sizeof(*img.grid)),
                          .used = malloc(256 * sizeof(bool)),
                          .grid_height = 45 * 8,
                          .grid_width = 60 * 8};
  int read = 0;

  for (size_t bucket = 0; bucket < 10; bucket++) {
    Color_t c = color_from_intensity((bucket / (double)10) + 0.1,
                                     (bucket / (double)10) + 0.1,
                                     (bucket / (double)10) + 0.1);
    for (size_t b = 0; b < (HEIGHT * WIDTH + 7) / 8; b++) {
      uint8_t byte;
      fread(&byte, sizeof(byte), 1, fp);
      read += 1;
      for (size_t itr = 0; itr < 8; itr++) {
        if ((byte >> (7 - itr)) & 0x1) {
          for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
              image_add_pixel(&img, c, 8 * ((b * 8 + itr) / WIDTH) + j,
                              8 * ((b * 8 + itr) % WIDTH) + k);
            }
          }
        }
      }
    }
  }

  printf("\033[H\n");
  puts(argv[1]);
  printf("\033Pq\n");
  color_register_setup(stdout);
  print_sixel(stdout, &img);
  color_register_teardown(stdout);
  puts("");
  printf("\033\\\n\n");
  usleep(60000);
}
