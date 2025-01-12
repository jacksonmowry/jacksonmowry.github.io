#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define NUM_CACHED 400000

typedef struct entry {
  int64_t key;
  int64_t val;
  int64_t step;
  struct entry *next;
} entry_t;

typedef struct hashmap {
  entry_t **arr;
} hashmap_t;

hashmap_t hashmap_init() {
  return (hashmap_t){.arr = calloc(sizeof(entry_t *), NUM_CACHED)};
}

void hashmap_insert(hashmap_t *h, int64_t key, int64_t val, int64_t step) {
  entry_t *e = malloc(sizeof(entry_t));
  *e = (entry_t){
      .key = key, .val = val, .step = step, .next = h->arr[key % NUM_CACHED]};
  h->arr[key % NUM_CACHED] = e;
}

entry_t *hashmap_find(hashmap_t *h, int64_t key, int64_t step) {
  entry_t *e = h->arr[key % NUM_CACHED];

  while (e) {
    if (e->key == key && e->step == step) {
      return e;
    }

    e = e->next;
  }

  return NULL;
}

size_t total_stones = 0;

int num_digits(uint64_t num) {
  int digits = 0;

  while (num) {
    digits++;
    num /= 10;
  }

  return digits;
}

<<<<<<< Updated upstream
hashmap_t h;
=======
#define NUM_BLINKS 75
#define NUM_CACHED 80000000
int64_t *cache;
#define cache_at(steps, num) cache[(steps * NUM_CACHED) + num]
>>>>>>> Stashed changes

uint64_t recurse(uint64_t num, uint64_t steps) {
  if (steps == 75) {
    return 1;
  }

  entry_t *e;
  if ((e = hashmap_find(&h, num, steps))) {
    return e->val;
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

  hashmap_insert(&h, num, inner_count, steps);

  return inner_count;
}

int main() {
  h = hashmap_init();
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
