#include <assert.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define HEAP_INC 32

void *heap_base = NULL;
void *heap_end = NULL;

typedef struct {
  size_t size;
} heap_seg;

void *malloc(size_t bytes) {
  if (!heap_base) {
    heap_base = sbrk(0);
    sbrk(HEAP_INC);
    *(heap_seg*)heap_base = (heap_seg){HEAP_INC};
    heap_end = sbrk(0);
  }
  if (bytes == 0) {
    return NULL;
  }

  // Start from heap base, find first open segment that can hold 'bytes'
  heap_seg *ptr = heap_base;
  while (ptr < (heap_seg*)heap_end) {
    /* printf("Size is: %ld, Free?: %s\n", ptr->size ^ 0x1, */
    /*        ((ptr->size & 1) == 0) ? "Yes" : "No"); */
    if ((ptr->size & 0x1) == 0 && ptr->size >= bytes + sizeof(heap_seg)) {
      // Store old capacity
      size_t old_cap = ptr->size;
      // Heap segment is free, and big enough
      ptr->size = sizeof(heap_seg) +
                      ((bytes + (sizeof(heap_seg) - 1)) / sizeof(heap_seg)) *
                          sizeof(heap_seg) |
                  0x1;
      void *user_block = ptr + 1;
      size_t next_size = old_cap - (ptr->size ^ 0x1);
      if (next_size <= sizeof(heap_seg)) {
        ptr->size += next_size;
      } else if (old_cap > ptr->size) {
        heap_seg *next_block = ptr + ptr->size / sizeof(heap_seg);
        next_block->size = old_cap - (ptr->size ^ 0x1);
        /* printf("next blocks size: %ld\n", next_block->size); */
      }
      return user_block;
    }
    ptr += ptr->size / sizeof(heap_seg);
  }

  // Ran out of space
  /* puts("RESIZING"); */
  size_t increment = (bytes > HEAP_INC) ?
    sizeof(heap_seg) + ((bytes + (sizeof(heap_seg)-1)) / sizeof(heap_seg)) * sizeof(heap_seg)
    : HEAP_INC;
  sbrk(increment);
  heap_end = sbrk(0);
  size_t new_size = sizeof(heap_seg) + ((bytes+(sizeof(heap_seg)-1))/sizeof(heap_seg))*sizeof(heap_seg)|0x1;
  ptr->size = new_size;
  size_t next_size = increment - (ptr->size ^ 0x1);
  if (next_size <= sizeof(heap_seg)) {
    ptr->size += next_size;
  } else if (bytes < HEAP_INC) {
    heap_seg *next_block = ptr + ptr->size / sizeof(heap_seg);
    next_block->size = next_size;
    /* printf("next blocks size: %ld\n", next_block->size); */
  }
  return ptr + 1;

  // return NULL;
}

void free(void* block) {
  if (!block) { return; }
  heap_seg *header = (heap_seg*)block - 1;
  // Get prev/next pointers
  heap_seg *prev = header - header->size / sizeof(heap_seg);
  heap_seg *next = header + header->size / sizeof(heap_seg);
  if (prev > (heap_seg*)heap_base && !(prev->size & 1)) {
    prev->size += header->size;
    header = prev;
  }
  if (next < (heap_seg*)heap_end && !(next->size & 1)) {
    header->size += next->size;
  }

  header->size ^= 1;
}

void heap_size() {
  /* printf("Heap is %ld bytes\n", heap_end - heap_base); */
}

int main() {
  int *block = malloc(4);
  *block = 4;
  heap_size();

  int *num = malloc(4);
  *num = 7;
  heap_size();

  int *test = malloc(1);
  *test = 66;
  heap_size();

  /* printf("Address of block: %p\n", block); */
  /* printf("Address of num: %p\n", num); */
  /* printf("Address of test: %p\n", test); */

  /* printf("Size of block: %ld\n", ((heap_seg*)block-1)->size ^ 0x1); */
  /* printf("Size of num: %ld\n", ((heap_seg*)num-1)->size ^ 0x1); */
  /* printf("Size of test: %ld\n", ((heap_seg*)test-1)->size ^ 0x1); */

  free(num);
  free(test);

  num = malloc(32);
  *num = 1;
  heap_size();

  uint64_t *big_num = malloc(64);
  *big_num = 64;
  heap_size();
  /* printf("address of big_num: %p\n", big_num); */
  /* printf("big_num: %ld\n", *big_num); */

  /* printf("Address of block: %p\n", block); */
  /* printf("Address of num: %p\n", num); */
  /* printf("Size of block: %ld\n", ((heap_seg*)block-1)->size ^ 0x1); */
  /* printf("Size of num: %ld\n", ((heap_seg*)num-1)->size ^ 0x1); */

  heap_seg* ptr = heap_base;
  while (ptr < (heap_seg*)heap_end) {
    printf("Segment size: %ld, Value: %ld\n", ptr->size, (ptr->size & 1) ? *(uint64_t*)(ptr+1) : -1);
    ptr += ptr->size / sizeof(heap_seg);
  }


  return 0;
}
