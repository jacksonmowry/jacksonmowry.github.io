#pragma once
#include <stdint.h>

#define TOMBSTONE UINT64_MAX

bool is_prime(int);
int next_prime(int);

typedef struct {
  void *key;
  uint64_t value;
} pair;

// Hashing Functions
typedef uint64_t (*hash_function)(void *);
uint64_t no_hash(void *);
uint64_t basic_hash(void *);
uint64_t fnv_hash(void *);

//Hashmap
typedef struct {
  float load_factor;
  int capacity;
  int size;
  int collisions;
  hash_function hash_func;
  pair *array;
} hashmap;

hashmap hashmap_init(hash_function, float, int);
void hashmap_free(hashmap*);
void hashmap_resize(hashmap*, int);
void hashmap_insert(hashmap*, void*, uint64_t);
uint64_t hashmap_get(hashmap*, void*);
uint64_t hashmap_delete(hashmap*, void*);
void hashmap_print(hashmap*);
