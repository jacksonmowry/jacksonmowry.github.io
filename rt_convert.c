#include <stdint.h>
#include <stdio.h>

int main() {
  printf("P3\n");

  uint16_t width;
  uint16_t height;
  fread(&width, 2, 1, stdin);
  fread(&height, 2, 1, stdin);

  printf("%d %d\n", width, height);

  printf("255\n");

  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      uint8_t r, g, b;
      fread(&r, 1, 1, stdin);
      fread(&g, 1, 1, stdin);
      fread(&b, 1, 1, stdin);

      printf("%d %d %d ", r, g, b);
    }
    putchar('\n');
  }

  putchar('\n');
}
