#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "gallocc.h"

size_t allocations = 0;

int main() {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);
  char *ptr;
  for (int i = 1; i < 30000; i++) {
    allocations++;
    ptr = malloc(i);
    *ptr = i;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0 +
    (end.tv_nsec - start.tv_nsec) / 1e6;
  printf("Time: %.3f milliseconds\n", elapsed_time);

  /* heap_walk(); */
}
