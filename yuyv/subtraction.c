#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HEIGHT 480
#define WIDTH 640
#define THRESHOLD 40

uint8_t img1[HEIGHT][WIDTH];
uint8_t img2[HEIGHT][WIDTH];

int main() {
  char junk[1024];
  int w;
  fscanf(stdin, "%s", junk);      // P2
  fscanf(stdin, "%d %d", &w, &w); // w h
  fscanf(stdin, "%d", &w);        // intensity

  for (int row = 0; row < HEIGHT; row++) {
    for (int col = 0; col < WIDTH; col++) {
      fscanf(stdin, "%hhu", &img1[row][col]);
    }
  }

  fscanf(stdin, "%s", junk);      // P2
  fscanf(stdin, "%d %d", &w, &w); // w h
  fscanf(stdin, "%d", &w);        // intensity

  for (int row = 0; row < HEIGHT; row++) {
    for (int col = 0; col < WIDTH; col++) {
      fscanf(stdin, "%hhu", &img2[row][col]);
    }
  }

  printf("P2\n");
  printf("%d %d\n", WIDTH, HEIGHT);
  printf("255\n");

  for (int row = 0; row < HEIGHT; row++) {
    for (int col = 0; col < WIDTH; col++) {
      if (abs(img1[row][col] - img2[row][col]) > THRESHOLD) {
        printf("255 ");
      } else {
        printf("0 ");
      }
    }
    putchar('\n');
  }
  putchar('\n');
}
