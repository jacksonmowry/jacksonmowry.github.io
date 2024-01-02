#include <assert.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#define HEAP_SIZE 1024

void *heap_base;

typedef struct {
  size_t size;
} heap_seg;

int heap_init(void *base) {
  if (!base) {
    return -1;
  }
  heap_base = base;

  *(heap_seg *)heap_base = (heap_seg){HEAP_SIZE - sizeof(heap_seg)};
  return 0;
}

void *heap_malloc(size_t bytes) {
  if (bytes == 0) {
    return NULL;
  }
  if (bytes > HEAP_SIZE) {
    printf("Too big of allocation\nRequested: %ld, Capacity: %d\n", bytes,
           HEAP_SIZE);
  }

  // Start from heap base, find first open segment that can hold 'bytes'
  heap_seg *ptr = heap_base;
  while (ptr < (heap_seg *)(heap_base + HEAP_SIZE)) {
    printf("Size is: %ld, Free?: %s\n", ptr->size,
           ((ptr->size & 1) == 0) ? "Yes" : "No");
    if ((ptr->size & 0x1) == 0 && ptr->size >= bytes + sizeof(heap_seg)) {
      // Store old capacity
      size_t old_cap = ptr->size;
      // Heap segment is free, and big enough
      ptr->size = sizeof(heap_seg) +
                      ((bytes + (sizeof(heap_seg) - 1)) / sizeof(heap_seg)) *
                          sizeof(heap_seg) |
                  0x1;
      printf("%ld\n", ptr->size);
      void *user_block = (void *)(ptr + 1);
      if (old_cap > ptr->size) {
        heap_seg *next_block = ptr + ptr->size / sizeof(heap_seg);
        next_block->size = old_cap - (ptr->size ^ 0x1);
        printf("next blocks size: %ld\n", next_block->size);
      }
      return user_block;
    }
    ptr += ptr->size / sizeof(heap_seg);
  }

  return NULL;
}

void heap_free(void* block) {
  if (!block) { return; }
  heap_seg *header = (heap_seg*)block - 1;
  // Get prev/next pointers
  heap_seg *prev = header - header->size / sizeof(heap_seg);
  heap_seg *next = header + header->size / sizeof(heap_seg);
  if (prev > (heap_seg*)heap_base && !(prev->size & 1)) {
    prev->size += header->size;
    header = prev;
  }
  if (next < heap_base + HEAP_SIZE && !(next->size & 1)) {
    header->size += next->size;
  }

  header->size ^= 1;
}

int main() {
  uint8_t heap[HEAP_SIZE];
  heap_init(heap);

  int *block = heap_malloc(4);
  *block = 4;

  int *num = heap_malloc(4);
  *num = 7;

  int *test = heap_malloc(4);
  *test = 66;

  printf("Address of block: %p\n", block);
  printf("Address of num: %p\n", num);
  printf("Address of test: %p\n", test);

  printf("Size of block: %d\n", ((heap_seg*)block-1)->size ^ 0x1);
  printf("Size of num: %d\n", ((heap_seg*)num-1)->size ^ 0x1);
  printf("Size of test: %d\n", ((heap_seg*)test-1)->size ^ 0x1);

  heap_free(num);
  heap_free(test);

  num = heap_malloc(32);
  *num = 1;

  printf("Address of block: %p\n", block);
  printf("Address of num: %p\n", num);
  printf("Size of block: %d\n", ((heap_seg*)block-1)->size ^ 0x1);
  printf("Size of num: %d\n", ((heap_seg*)num-1)->size ^ 0x1);
}
