#include "hashmap.h"
#include <assert.h>
#include <immintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

bool str_equals(void* a, void* b) { return false; };

uint64_t fast_int64_hash(uint64_t key) {
    key = (~key) + (key << 21); // key = (key << 21) - key - 1;
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key;
}

uint64_t fnv_string_hash(void* pointer) {
    if (!pointer) {
        return 0;
    }
    uint64_t hash = 14695981039346656037ull;
    const char* str = (const char*)pointer;
    while (*str) {
        hash *= 1099511628211;
        hash ^= *str++;
    }
    return hash;
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

    pair* array = calloc(capacity, sizeof(pair));
    if (!array) {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }
    return (hashmap){load_factor, capacity, 0, 0, hash, eq, array};
}

// Frees all dynamically associated memory with a hashmap
// the hashmap becomes invalid after calling free
void hashmap_free(hashmap* h) {
    if (!h) {
        return;
    }
    free(h->array);
    h = NULL;
}

int unsafe_strcmp(const char* s1, const char* s2) {
    while (*s1 == *s2) {
        if (*s1 == '\0')
            return 0;
        s1++;
        s2++;
    }
    return 1;
}

// Returns a pointer to the pair if found,
// otherwise returns NULL
pair* hashmap_find(hashmap* h, void* key) {
    uint64_t index = h->hash_func(key) % h->capacity;
    for (int i = 0; i < h->capacity / 2 + 1; i++) {
        /* uint64_t new_index = (index + (i * i)) % h->capacity; */
        // uint64_t new_index = (index + (i)) % h->capacity;
        pair* p = &h->array[index];
        if (p->key == 0) {
            h->collisions += i;
            char* copy_key = strdup(key);
            *p = (pair){.key = copy_key,
                        .value = (Record){
                            .name = key,
                            .total = 0,
                            .count = 0,
                            .min = INT64_MAX,
                            .max = INT64_MIN,
                        }};
            return p;
        } else if (!unsafe_strcmp(p->key, key)) {
            return p;
        }
        index = fast_int64_hash(index) % h->capacity;
    }
    return NULL;
}
