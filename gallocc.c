#include <assert.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HEAP_INC 1024

typedef struct heap_seg {
  size_t size;
  struct heap_seg *prev;
  struct heap_seg *next_free;
} heap_seg;

void *heap_base = NULL;
void *heap_end = NULL;
heap_seg *first_free = NULL;

size_t round_bytes(size_t bytes) {
  return (bytes + (sizeof(void *) - 1)) / sizeof(void *) * sizeof(void *);
}

bool segment_free(heap_seg *seg) { return !(seg->size & 0x1); }

size_t segment_size(heap_seg *seg) { return seg->size & (~1); }

/* void update_explicit(heap_seg *current, heap_seg *prev, heap_seg *next) { */
/*   bool forward_search = false; */
/*   // Find previous free segment */
/*   if (prev) { */
/*     prev = prev->prev; */
/*   } */
/*   while (prev && !segment_free(prev) && prev > (heap_seg *)heap_base) { */
/*     prev = prev->prev; */
/*   } */
/*   if (prev && segment_free(prev)) { */
/*     prev->next_free = current; */
/*   } else { */
/*     // We might now be first_free */
/*     if (segment_free(current)) { */
/*       first_free = current; */
/*     } else { */
/*       // Scan forward for free bock */
/*       forward_search = true; */
/*     } */
/*   } */
/*   // Find next free segment */
/*   next = (void *)next + segment_size(next); */
/*   while (!segment_free(next) && next < (heap_seg *)heap_end) { */
/*     next = (void *)next + segment_size(next); */
/*   } */
/*   if (segment_free(next)) { */
/*     current->next_free = next; */
/*     if (forward_search) { first_free = next; } */
/*   } else { */
/*     current->next_free = NULL; */
/*   } */
/* } */

int heap_init(size_t bytes) {
  heap_base = sbrk(0);
  first_free = heap_base;
  size_t increment =
      (bytes > HEAP_INC) ? sizeof(heap_seg) + round_bytes(bytes) : HEAP_INC;
  sbrk(HEAP_INC);
  *(heap_seg *)heap_base = (heap_seg){HEAP_INC, NULL, NULL};
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
  heap_seg *ptr = first_free;

  heap_seg *best = NULL;
  while (ptr && ptr < (heap_seg *)heap_end) {
    if (segment_free(ptr) && ptr->size >= bytes + sizeof(heap_seg) &&
        (!best || segment_size(ptr) < segment_size(best))) {
      best = ptr;
    }
    if (!ptr->next_free) {
      break;
    }
    ptr = ptr->next_free;
  }

  // Found a suitable block for Best-Fit
  if (best) {
    ptr = best;
    // Store old capacity
    size_t old_cap = ptr->size;
    // Heap segment is free, and big enough
    ptr->size = sizeof(heap_seg) + round_bytes(bytes) | 0x1;
    void *user_block = ptr + 1;
    size_t next_size = old_cap - segment_size(ptr);
    if (next_size <= sizeof(heap_seg)) {
      ptr->size += next_size;
    } else if (old_cap > ptr->size) {
      // Scan for previous free block
      heap_seg *prev_free = NULL;
      if (best) {
        prev_free = best->prev;
      }
      while (prev_free && !segment_free(prev_free) &&
             prev_free >= (heap_seg *)heap_base) {
        prev_free = prev_free->prev;
      }

      heap_seg *next_block = (void *)ptr + segment_size(ptr);
      next_block->size = old_cap - segment_size(ptr);
      next_block->prev = ptr;
      next_block->next_free = ptr->next_free;
      heap_seg *next_next_block = (void *)next_block + segment_size(next_block);
      next_next_block->prev = next_block;
      if (prev_free) {
        prev_free->next_free = next_block;
      } else {
        first_free = next_block;
      }
    }
    return user_block;
  }

  // Ran out of space
  size_t next_size;
  size_t increment = (bytes > HEAP_INC) ? round_bytes(bytes) : HEAP_INC;
  size_t new_size = round_bytes(bytes) | 0x1;

  heap_seg *new_seg = (void *)ptr + segment_size(ptr);
  new_seg->prev = ptr;
  new_seg->next_free = NULL;

  if (new_seg->prev && segment_free(new_seg->prev)) {
    new_seg = new_seg->prev;
    sbrk(increment);
    size_t old_size = new_seg->size;
    new_seg->size = new_size + sizeof(heap_seg);
    next_size = (old_size + increment) - segment_size(new_seg);
  } else {
    // Previous chunk is not free
    sbrk(increment + sizeof(heap_seg));
    new_seg->size = new_size + sizeof(heap_seg);
    next_size = increment - segment_size(new_seg);
  }
  heap_end = sbrk(0);
  // Check if the next segment should be initialized
  if (next_size <= sizeof(heap_seg)) {
    ptr->size += next_size;
  } else if (next_size > 0) {
    // Scan for previous free block
    heap_seg *prev_free = new_seg;
    while (prev_free && !segment_free(prev_free) &&
           prev_free >= (heap_seg*)heap_base) {
      prev_free = prev_free->prev;
    }

    heap_seg *next_block = (void *)ptr + segment_size(ptr);
    next_block->size = next_size;
    next_block->prev = ptr;
    if (prev_free) {
      prev_free->next_free = next_block;
    } else {
      first_free = next_block;
    }
    next_block->next_free = NULL;
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

  // Scan backwards for free segment
  if (prev) {
    prev = prev->prev;
  }
  while (prev && !segment_free(prev) && prev >= (heap_seg *)heap_base) {
    prev = prev->prev;
  }
  if (prev) {
    header->next_free = prev->next_free;
    prev->next_free = header;
  } else {
    header->next_free = first_free;
    first_free = header;
  }
}

void heap_size() { printf("Heap is %ld bytes\n", heap_end - heap_base); }

void heap_walk() {
  heap_seg *ptr = heap_base;
  while (ptr < (heap_seg *)heap_end) {
    printf("Segment size: %4ld, Prev: %14p, Value: %3d, Address: %p",
           ptr->size, ptr->prev, (ptr->size & 1) ? *(int8_t *)(ptr + 1) : (int8_t)-1,
           ptr);
    if (segment_free(ptr)) {
      printf(" --> Next Free: %p\n", ptr->next_free);
    } else {
      printf("\n");
    }
    ptr = (void *)ptr + (ptr->size & (~1));
  }
}

int main() {
  int8_t *block = malloc(244);
  *block = 4;

  int8_t *num = malloc(244);
  *num = 7;

  int8_t *test = malloc(244);
  *test = 66;

  int8_t *big_num = malloc(244);
  *big_num = 64;

  /* free(block); */
  free(test);

  int8_t *block2 = malloc(16);
  *block2 = 9;
  test = malloc(224);
  *test = 2;

  int8_t *where = malloc(8);
  *where = 24;

  char *buf;
  asprintf(&buf, "Testing all our values:\n%d\n%d\n%d\n%d\n%d",
           *num, *test, *big_num, *block2, *where);
  puts(buf);


  free(block);
  heap_walk();
  printf("First free: %p\n", first_free);
  exit(1);
  return 0;
}
