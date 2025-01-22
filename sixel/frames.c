#include "color.h"
#include "image.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {

  FILE *fp = fopen("frames.txt", "r");

  for (int frame = 0; frame < 84; frame++) {
    /* Image_t img = (Image_t){.grid = malloc(260 * 340 * sizeof(*img.grid)), */
    /*                         .used = malloc(256 * sizeof(bool)), */
    /*                         .grid_height = 260, */
    /*                         .grid_width = 340}; */
    for (int i = 0; i < 260; i++) {
      for (int j = 0; j < 346; j++) {
        int d;
        fscanf(fp, "%d", &d);
        if (d) {
          /* image_add_pixel(&img, (Color_t){.r = 255}, j, i); */
          printf("8");
        } else {
          printf(" ");
        }
      }
      putchar('\n');
    }

    /* printf("\033[H\n"); */
    /* printf("\033Pq\n"); */
    /* color_register_setup(stdout); */
    /* print_sixel(stdout, &img); */

    /* color_register_teardown(stdout); */
    /* puts(""); */
    /* printf("\033\\\n\n"); */
    usleep(50000);
    printf("\033[H");
  }
}
