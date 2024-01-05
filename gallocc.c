#include <assert.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HEAP_INC 1024

void *heap_base = NULL;
void *heap_end = NULL;

typedef struct heap_seg {
  size_t size;
  struct heap_seg *prev;
} heap_seg;

size_t round_bytes(size_t bytes) {
  return (bytes + (sizeof(void *) - 1)) / sizeof(void *) * sizeof(void *);
}

bool segment_free(heap_seg *seg) { return !(seg->size & 0x1); }

size_t segment_size(heap_seg *seg) { return seg->size & (~1); }

int heap_init(size_t bytes) {
    heap_base = sbrk(0);
    size_t increment =
        (bytes > HEAP_INC) ? sizeof(heap_seg) + round_bytes(bytes) : HEAP_INC;
    sbrk(HEAP_INC);
    *(heap_seg *)heap_base = (heap_seg){HEAP_INC, NULL};
    if ((int64_t)(heap_end = sbrk(0)) == -1) {
      return -1;
    }
    return 0;
}

void *malloc(size_t bytes) {
  if (!heap_base) {
    heap_init(bytes);
  }
  if (bytes == 0) {
    return NULL;
  }

  // Start from heap base, find first open segment that can hold 'bytes'
  heap_seg *prev = NULL;
  heap_seg *ptr = heap_base;
  while (ptr < (heap_seg *)heap_end) {
    if (segment_free(ptr) && ptr->size >= bytes + sizeof(heap_seg)) {
      // Store old capacity
      size_t old_cap = ptr->size;
      // Heap segment is free, and big enough
      ptr->size = sizeof(heap_seg) + round_bytes(bytes) | 0x1;
      ptr->prev = prev;
      void *user_block = ptr + 1;
      size_t next_size = old_cap - segment_size(ptr);
      if (next_size <= sizeof(heap_seg)) {
        ptr->size += next_size;
      } else if (old_cap > ptr->size) {
        heap_seg *next_block = (void *)ptr + segment_size(ptr);
        next_block->size = old_cap - segment_size(ptr);
        next_block->prev = ptr;
      }
      return user_block;
    }
    prev = ptr;
    ptr = (void *)ptr + segment_size(ptr);
  }

  // Ran out of space
  size_t next_size;
  size_t increment = (bytes > HEAP_INC) ? round_bytes(bytes) : HEAP_INC;
  size_t new_size = round_bytes(bytes) | 0x1;

  if (segment_free(prev)) {
    ptr = prev;
    prev = prev->prev;
    sbrk(increment);
    size_t old_size = ptr->size;
    ptr->size = new_size + sizeof(heap_seg);
    next_size = (old_size + increment) - segment_size(ptr);
  } else {
    // Previous chunk is not free
    sbrk(increment + sizeof(heap_seg));
    ptr->size = new_size + sizeof(heap_seg);
    ptr->prev = prev;
    next_size = increment - segment_size(ptr);
  }
  heap_end = sbrk(0);
  // Check if the next segment should be initialized
  if (next_size <= sizeof(heap_seg)) {
    ptr->size += next_size;
  } else if (next_size > 0) {
    heap_seg *next_block = (void *)ptr + segment_size(ptr);
    next_block->size = next_size;
    next_block->prev = ptr;
  }
  // Return user writable address
  return ptr + 1;
  // return NULL;
}

void free(void *block) {
  if (!block) {
    return;
  }
  heap_seg *header = (heap_seg *)block - 1;
  // Get prev/next pointers
  heap_seg *prev = header->prev;
  heap_seg *next = (void *)header + segment_size(header);
  if (prev > (heap_seg *)heap_base && segment_free(prev)) {
    prev->size += header->size;
    header = prev;
  }
  if (next < (heap_seg *)heap_end && segment_free(next)) {
    header->size += next->size;
  }
  header->size ^= 1;
}

void heap_size() { printf("Heap is %ld bytes\n", heap_end - heap_base); }

void heap_walk() {
  heap_seg *ptr = heap_base;
  while (ptr < (heap_seg *)heap_end) {
    printf("Segment size: %4ld, Prev: %14p, Value: %19ld, Address: %p\n",
           ptr->size, ptr->prev, (ptr->size & 1) ? *(uint64_t *)(ptr + 1) : -1,
           ptr);
    ptr = (void *)ptr + (ptr->size & (~1));
  }
}

int main() {
  int *block = malloc(4);
  *block = 4;

  uint64_t *num = malloc(8);
  *num = 7;

  uint64_t *test = malloc(69);
  *test = 66;

  uint64_t *big_num = malloc(64);
  *big_num = 64;

  char *buf;
  asprintf(&buf, "big num has the value: %ld\n", *big_num);
  puts(buf);

  heap_walk();
  return 0;
}
