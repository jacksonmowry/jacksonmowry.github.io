#include "hashmap.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gallocc.h"

bool is_prime(int num) {
  if (num < 2) {
    return false;
  }
  for (int i = 2; i * i <= num; i++) {
    if (num % i == 0) {
      return false;
    }
  }

  return true;
}

int next_prime(int current_prime) {
  int next = current_prime + 1;
  while (!is_prime(next)) {
    next++;
  }
  return next;
}

uint64_t no_hash(void *pointer) { return 0; }

uint64_t basic_hash(void *pointer) {
  return (uint64_t)pointer ^ ((uint64_t)pointer >> 32);
}

uint64_t fnv_hash(void *pointer) {
  // Prime and offset basis values for 64-bit values
  uint64_t hash = 14695981039346656037ull;
  uint64_t byte_pointer = (uint64_t)pointer;
  for (int i = 0; i < 8; i++) {
    hash *= 1099511628211;
    hash ^= byte_pointer & 0xFF;
    byte_pointer >>= 8;
  }
  return hash;
}

uint64_t fnv_string_hash(void *pointer) {
  if (!pointer) {
    return 0;
  }
  uint64_t hash = 14695981039346656037ull;
  const char *str = (const char *)pointer;
  while (*str) {
    hash *= 1099511628211;
    hash ^= *str++;
  }
  return hash;
}

// Compares 2 8-byte values
bool value_equals(void *a, void *b) { return a == b; }

// Compares 2 null terminated C-style strings
bool str_equals(void *a, void *b) {
  return strcmp((const char *)a, (const char *)b) == 0;
}

// Writes at most STR_BUF_LEN bytes into buf,
// can be done in any way, but snprintf is the easiest
void unsigned_print(char *buf, void *value) {
  snprintf(buf, STR_BUF_LEN, "%lu", (uint64_t)value);
}
void signed_print(char *buf, void *value) {
  snprintf(buf, STR_BUF_LEN, "%ld", (int64_t)value);
}
void double_print(char *buf, void *value) {
  snprintf(buf, STR_BUF_LEN, "%f", *(double *)value);
}
void address_print(char *buf, void *value) {
  snprintf(buf, STR_BUF_LEN, "%p", value);
}
void string_print(char *buf, void *str) {
  snprintf(buf, STR_BUF_LEN, "%s", (char *)str);
}

// `hash` is any hash function that takes a `void*` and returns a `uint64_t`
// Set `load_factor` to `0` for default value
// Set `size` to `0` for default value
hashmap hashmap_init(hash_function hash, equals_function eq, float load_factor,
                     int capacity) {
  if (!hash) {
    fprintf(stderr, "Please specify a valid hash function\n");
    exit(EXIT_FAILURE);
  }
  if (!load_factor) {
    load_factor = 0.5;
  }
  if (!capacity) {
    capacity = 11;
  }

  pair *array = calloc(capacity, sizeof(pair));
  if (!array) {
    perror("calloc failed");
    exit(EXIT_FAILURE);
  }
  return (hashmap){load_factor, capacity, 0, 0, hash, eq, array};
}

// Frees all dynamically associated memory with a hashmap
// the hashmap becomes invalid after calling free
void hashmap_free(hashmap *h) {
  if (!h) {
    return;
  }
  free(h->array);
  h = NULL;
}

// Returns a pointer to the pair if found,
// otherwise returns NULL
pair *hashmap_find(hashmap *h, void *key) {
  uint64_t index = h->hash_func(key) % h->capacity;
  for (int i = 0; i < h->capacity / 2 + 1; i++) {
    uint64_t new_index = (index + (i * i)) % h->capacity;
    pair p = h->array[new_index];
    switch ((uint64_t)p.key) {
    case 0:
      return NULL;
    case TOMBSTONE:
      continue;
    default:
      if (h->equals_func(p.key, key)) {
        return &h->array[new_index];
      }
    }
  }
  return NULL;
}

// Returns a pointer to the first available spot,
// empty or TOMBSTONE
pair *hashmap_first_avail(hashmap *h, void *key, int *collisions) {
  uint64_t index = h->hash_func(key) % h->capacity;
  for (int i = 0; i < h->capacity / 2 + 1; i++) {
    uint64_t new_index = (index + (i * i)) % h->capacity;
    pair p = h->array[new_index];
    switch ((uint64_t)p.key) {
    case 0:
      *collisions = i;
      return &h->array[new_index];
    case TOMBSTONE:
      *collisions = i;
      return &h->array[new_index];
    default:
      continue;
    }
  }
  return NULL;
}

// Set `new_cap` to NULL automatically find the best table size
// `new_cap` should be set to a prime number for best performace
void hashmap_resize(hashmap *h, int new_cap) {
  if (!new_cap) {
    new_cap = next_prime(h->capacity);
  }

  int old_cap = h->capacity;
  pair *old_array = h->array;

  h->capacity = new_cap;
  h->collisions = 0;
  h->array = calloc(new_cap, sizeof(pair));
  if (!h->array) {
    perror("calloc failed");
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < old_cap; i++) {
    // Loop through all elements
    if ((uint64_t)old_array[i].key != 0 &&
        (uint64_t)old_array[i].key != TOMBSTONE) {
      // If we have an element, place it into the new array, without worrying
      // about tombstones
      uint64_t index = h->hash_func(old_array[i].key) % h->capacity;
      for (int j = 0;; j++) {
        // No termination condition, we must find a spot
        uint64_t new_index = (index + (j * j)) % h->capacity;
        if (h->array[new_index].key == 0) {
          h->array[new_index] = old_array[i];
          h->collisions += j;
          break;
        }
      }
    }
  }
  free(old_array);
}

void hashmap_insert(hashmap *h, void *key, uint64_t value) {
  pair *p = hashmap_find(h, key);
  pair new = {key, value};

  // Key already exists
  if (p) {
    *p = new;
    return;
  }

  if (((float)h->size + 1) / (float)h->capacity > h->load_factor) {
    // We are over capacity
    hashmap_resize(h, 0);
    hashmap_insert(h, key, value);
    return;
  }

  int collisions;
  pair *first_avail = hashmap_first_avail(h, key, &collisions);

  h->size++;
  h->collisions += collisions;

  *first_avail = new;
}

// Returns TOMBSTONE if element is not found
// returns value if found
uint64_t hashmap_get(hashmap *h, void *key) {
  pair *p = hashmap_find(h, key);
  if (!p) {
    return TOMBSTONE;
  } else {
    return p->value;
  }
}

// Returns TOMBSTONE if element is not found
// returns value if found
uint64_t hashmap_delete(hashmap *h, void *key) {
  pair *p = hashmap_find(h, key);
  if (!p) {
    return TOMBSTONE;
  } else {
    uint64_t tmp_val = p->value;
    h->size--;
    p->key = (void *)TOMBSTONE;
    p->value = 0;
    return tmp_val;
  }
}

void hashmap_print(hashmap *h, print_function key_print,
                   print_function value_print) {
  printf("Size: %d\n", h->size);
  printf("Capacity: %d\n", h->capacity);
  printf("Collisions: %d\n", h->collisions);
  printf("Average Steps Per Element: %f\n", h->collisions / (float)h->size);
  printf("Desired Load Factor: %f\n", h->load_factor);
  printf("Current Load Factor: %f\n", (float)h->size / (float)h->capacity);
  for (int i = 0; i < h->capacity; i++) {
    pair p = h->array[i];
    char key[STR_BUF_LEN];
    char value[STR_BUF_LEN];
    switch ((uint64_t)p.key) {
    case 0:
      snprintf(key, STR_BUF_LEN, "%s", "EMPTY");
      snprintf(value, STR_BUF_LEN, "%s", "EMPTY");
      break;
    case TOMBSTONE:
      snprintf(key, STR_BUF_LEN, "%s", "TOMBSTONE");
      snprintf(value, STR_BUF_LEN, "%s", "TOMBSTONE");
      break;
    default:
      key_print(key, p.key);
      value_print(value, (void *)p.value);
      break;
    }

    printf("Index: %3d, Key: %16s, Value: %16s\n", i, key, value);
  }
}

int main() {
  hashmap h = hashmap_init(&fnv_string_hash, &str_equals, 0, 0);

  char *billy = "Billy";
  char *Bailey = "Bailey";
  char *josh = "Josh";
  char *dylan = "Dylan";
  char *steve = "Steve";
  char *jackson = "Jackson";

  hashmap_insert(&h, billy, 25);
  hashmap_insert(&h, Bailey, 92);
  hashmap_insert(&h, josh, 11);
  hashmap_insert(&h, dylan, 17);
  hashmap_insert(&h, steve, 36);
  hashmap_insert(&h, jackson, 23);

  hashmap_print(&h, string_print, signed_print);

  hashmap_free(&h);
}
