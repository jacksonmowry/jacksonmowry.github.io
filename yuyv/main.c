#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HEIGHT 480
#define WIDTH 640

int main() {
  printf("P2\n");
  printf("%d %d\n", WIDTH, HEIGHT);
  printf("255\n");

  for (int row = 0; row < HEIGHT; row++) {
    for (int col = 0; col < WIDTH; col++) {
      uint8_t Y0, U, Y1, V;

      fread(&Y0, 1, 1, stdin);
      fread(&U, 1, 1, stdin);
      fread(&Y1, 1, 1, stdin);
      fread(&V, 1, 1, stdin);

      uint8_t r, g, b;
      uint8_t c, d, e;

      // Pixel 1
      c = Y0 - 16;
      d = U - 128;
      e = V - 128;

      r = Y0 + 1.402 * (V - 128);
      g = Y0 - 0.344136 * (U - 128) - 0.714136 * (V - 128);
      b = Y0 + 1.772 * (U - 128);

      r = r > 255 ? 255 : r;
      g = g > 255 ? 255 : g;
      b = b > 255 ? 255 : b;

      printf("%d ", (int)(0.299 * r + 0.587 * g + 0.114 * b));

      // Pixel 2
      c = Y1 - 16;

      r = Y0 + 1.402 * (V - 128);
      g = Y0 - 0.344136 * (U - 128) - 0.714136 * (V - 128);
      b = Y0 + 1.772 * (U - 128);

      r = r > 255 ? 255 : r;
      g = g > 255 ? 255 : g;
      b = b > 255 ? 255 : b;

      printf("%d ", (int)(0.299 * r + 0.587 * g + 0.114 * b));
    }
    putchar('\n');
  }

  putchar('\n');
}
