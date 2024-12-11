#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t total_stones = 0;

int num_digits(uint64_t num) {
  int digits = 0;

  while (num) {
    digits++;
    num /= 10;
  }

  return digits;
}

#define NUM_BLINKS 75
#define NUM_CACHED 50000000
int64_t *cache;
#define cache_at(steps, num) cache[(steps * NUM_CACHED) + num]

uint64_t recurse(uint64_t num, uint64_t steps) {
  if (steps == 75) {
    return 1;
  }

  if (cache_at(steps, num) != -1) {
    return cache_at(steps, num);
  }

  uint64_t inner_count = 0;

  if (num == 0) {
    inner_count += recurse(1, steps + 1);
  } else {
    int digits = num_digits(num);

    if (digits % 2 == 0) {
      size_t split = 1;
      for (size_t j = 0; j < digits / 2; ++j) {
        split *= 10;
      }
      uint64_t upper_half = num / split;
      uint64_t lower_half = num % split;

      inner_count += recurse(upper_half, steps + 1);
      inner_count += recurse(lower_half, steps + 1);
    } else {
      inner_count += recurse(num * 2024, steps + 1);
    }
  }

  cache_at(steps, num) = inner_count;

  return inner_count;
}

int main() {
  cache = malloc(sizeof(int64_t) * NUM_BLINKS * NUM_CACHED);
  memset(cache, -1, sizeof(int64_t) * NUM_BLINKS * NUM_CACHED);
  uint64_t input[10];
  size_t input_len = 0;

  while (scanf("%lu", &input[input_len++]) == 1) {
    ;
  }
  input_len--;

  for (int i = 0; i < input_len; i++) {
    printf("Processing stone %d\n", i);
    total_stones += recurse(input[i], 0);
  }

  printf("%zu\n", total_stones);
}
