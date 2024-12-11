#include <stdint.h>
#include <stdio.h>

int main() {
  uint64_t total = 0;
  uint64_t temp;

  while (scanf("%lu", &temp) == 1) {
    int64_t inner = (temp / 3) - 2;
    printf("adding %ld\n", inner);
    total += inner;

    int64_t fuel = inner;

    while (fuel > 0) {
      fuel = (fuel / 3) - 2;
      if (fuel < 0) {
        break;
      }
      printf("adding %ld\n", fuel);
      total += fuel;
    }
  }

  printf("%zu\n", total);
}
