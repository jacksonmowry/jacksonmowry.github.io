#include <assert.h>
#include <linux/limits.h>
#include <stdint.h>
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

void *malloc(size_t bytes) {
  if (!heap_base) {
    heap_base = sbrk(0);
    sbrk(HEAP_INC);
    *(heap_seg *)heap_base = (heap_seg){HEAP_INC, NULL};
    heap_end = sbrk(0);
    /* printf("Diff: %ld\n", heap_end - heap_base); */
  }
  if (bytes == 0) {
    return NULL;
  }

  // Start from heap base, find first open segment that can hold 'bytes'
  heap_seg *prev = NULL;
  heap_seg *ptr = heap_base;
  while (ptr < (heap_seg *)heap_end) {
    /* printf("Size is: %ld, Free?: %s\n", ptr->size ^ 0x1, */
    /*        ((ptr->size & 1) == 0) ? "Yes" : "No"); */
    if ((ptr->size & 0x1) == 0 && ptr->size >= bytes + sizeof(heap_seg)) {
      // Store old capacity
      size_t old_cap = ptr->size;
      // Heap segment is free, and big enough
      ptr->size =
          sizeof(heap_seg) + ((bytes + (sizeof(void *) - 1)) / sizeof(void *)) *
                                 sizeof(void *) |
          0x1;
      ptr->prev = prev;
      void *user_block = ptr + 1;
      size_t next_size = old_cap - (ptr->size ^ 0x1);
      if (next_size <= sizeof(heap_seg)) {
        ptr->size += next_size;
      } else if (old_cap > ptr->size) {
        heap_seg *next_block = (void *)ptr + (ptr->size & (~1));
        next_block->size = old_cap - (ptr->size ^ 0x1);
        next_block->prev = prev;
        /* printf("next blocks size: %ld\n", next_block->size); */
      }
      return user_block;
    }
    prev = ptr;
    ptr = (void *)ptr + (ptr->size & (~1));
  }

  // Ran out of space
  if (!(prev->size & 1)) {
    // TODO need to find a way to include previous chunk if it's free
    ptr = prev;
    prev = prev->prev;
    size_t increment =
        (bytes > HEAP_INC)
            ? ((bytes + (sizeof(void *) - 1)) / sizeof(void *)) * sizeof(void *)
            : HEAP_INC;
    sbrk(increment);
    heap_end = sbrk(0);
    size_t new_size =
        ((bytes + (sizeof(void *) - 1)) / sizeof(void *)) * sizeof(void *) |
        0x1;
    size_t old_size = ptr->size;
    ptr->size = new_size + sizeof(heap_seg);
    size_t next_size = (old_size + increment) - (ptr->size ^ 0x1);
    if (next_size <= sizeof(heap_seg)) {
      ptr->size += next_size;
    } else {
      heap_seg *next_block = (void *)ptr + (ptr->size & (~1));
      next_block->size = next_size;
      next_block->prev = prev;
      /* printf("next blocks size: %ld\n", next_block->size); */
    }
    return ptr + 1;
  } else {
    // Previous chunk is not free
    size_t increment =
        (bytes > HEAP_INC)
            ? sizeof(heap_seg) +
                  ((bytes + (sizeof(void *) - 1)) / sizeof(void *)) *
                      sizeof(void *)
            : HEAP_INC;
    sbrk(increment);
    heap_end = sbrk(0);
    size_t new_size =
        sizeof(heap_seg) +
            ((bytes + (sizeof(void *) - 1)) / sizeof(void *)) * sizeof(void *) |
        0x1;
    ptr->size = new_size;
    ptr->prev = prev;
    size_t next_size = increment - (ptr->size ^ 0x1);
    if (next_size <= sizeof(heap_seg)) {
      ptr->size += next_size;
    } else if (bytes < HEAP_INC) {
      heap_seg *next_block = (void *)ptr + (ptr->size & (~1));
      next_block->size = next_size;
      next_block->prev = prev;
      /* printf("next blocks size: %ld\n", next_block->size); */
    }
    return ptr + 1;
  }
  // return NULL;
}

void free(void *block) {
  if (!block) {
    return;
  }
  heap_seg *header = (heap_seg *)block - 1;
  // Get prev/next pointers
  heap_seg *prev = header->prev;
  heap_seg *next = (void *)header + (header->size & (~1));
  if (prev > (heap_seg *)heap_base && !(prev->size & 1)) {
    prev->size += header->size;
    header = prev;
  }
  if (next < (heap_seg *)heap_end && !(next->size & 1)) {
    header->size += next->size;
  }
  header->size ^= 1;
}

void heap_size() { printf("Heap is %ld bytes\n", heap_end - heap_base); }

void heap_walk() {
  heap_seg *ptr = heap_base;
  while (ptr < (heap_seg *)heap_end) {
    printf("Segment size: %4ld, Prev: %14p, Value: %ld, Address: %p\n",
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
  heap_size();
  return 0;
}
