#ifndef GALLOCC_H_
#define GALLOCC_H_

#include <stdlib.h>
#include <stdbool.h>

#define HEAP_INC 135168

typedef struct heap_seg {
  size_t size;
  struct heap_seg *prev;
  struct heap_seg *next_free;
} heap_seg;

size_t round_bytes(size_t);

bool segment_free(heap_seg*);
size_t segment_size(heap_seg*);

void update_back(heap_seg*, heap_seg*, bool);

int heap_init(size_t);
void *malloc(size_t);
void free(void*);
void *calloc(size_t, size_t);
void *realloc(void*, size_t);

void heap_walk();

#endif // GALLOCC_H_
