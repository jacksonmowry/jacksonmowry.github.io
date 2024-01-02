#include "stdio.h"
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

uint64_t fnv_hash(void *pointer) {
  // Prime and offset basis values for 64-bit values
  uint64_t hash = 14695981039346656037U;
  uint64_t byte_pointer = (uint64_t)pointer;
  for (int i = 0; i < 8; i++) {
    hash *= 1099511628211;
    hash ^= byte_pointer & 0xFF;
    byte_pointer >>= 8;
  }
  return hash;
}

uint64_t basic_hash(void *pointer) {
  return (uint64_t)pointer ^ ((uint64_t)pointer >> 32);
}

int main() {
  void* a = malloc(8);
  void* b = malloc(8);
  void* c = malloc(8);
  puts("Basic Hash");
  printf("Hashed: %p  |  [14]Array Index %lu\n",(void*)basic_hash(a), basic_hash(a) % 14);
  printf("Hashed: %p  |  [14]Array Index %lu\n",(void*)basic_hash(b), basic_hash(b) % 14);
  printf("Hashed: %p  |  [14]Array Index %lu\n",(void*)basic_hash(c), basic_hash(c) % 14);
  puts("FNV Hash");
  printf("Hashed: %p  |  [14]Array Index %lu\n",(void*)fnv_hash(a), fnv_hash(a) % 14);
  printf("Hashed: %p  |  [14]Array Index %lu\n",(void*)fnv_hash(b), fnv_hash(b) % 14);
  printf("Hashed: %p  |  [14]Array Index %lu\n",(void*)fnv_hash(c), fnv_hash(c) % 14);
}
