#include <stdlib.h>
#include <stdio.h>

int main() {
  int *a = malloc(32);
  *a = 1;
  int *b = malloc(32);
  *b = 2;

  printf("a: %p\n", a);
  printf("b: %p\n", b);
  printf("distance: %d\n", (void*)b - (void*)a);
}
