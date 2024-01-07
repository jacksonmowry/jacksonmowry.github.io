#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "gallocc.h"

heap_seg *heap_base = NULL;
heap_seg *heap_end = NULL;
heap_seg *first_free = NULL;

size_t round_bytes(size_t bytes) {
  return (bytes + (sizeof(void *) - 1)) / sizeof(void *) * sizeof(void *);
}

bool segment_free(heap_seg *seg) { return !(seg->size & 0x1); }

size_t segment_size(heap_seg *seg) { return seg->size & (~1); }

void update_back(heap_seg *cur, heap_seg *target, bool free_mode) {
  heap_seg *prev_free = NULL;
  if (cur) {
    prev_free = cur->prev;
  }
  while (prev_free && !segment_free(prev_free) && prev_free >= heap_base) {
    prev_free = prev_free->prev;
  }

  if (prev_free) {
    if (free_mode) {
      target->next_free = prev_free->next_free;
    }
    prev_free->next_free = target;
  } else {
    if (free_mode) {
      target->next_free = first_free;
    }
    first_free = target;
  }
}

int heap_init(size_t bytes) {
  heap_base = sbrk(0);
  first_free = heap_base;
  size_t increment =
      (bytes > HEAP_INC) ? sizeof(heap_seg) + round_bytes(bytes) : HEAP_INC;
  if ((sbrk(increment)) == (void *)-1) {
    return -1;
  }
  *heap_base = (heap_seg){increment, NULL, NULL};
  heap_end = sbrk(0);
  /* assert((void *)heap_end - (void *)heap_base == HEAP_INC); */
  return 0;
}

void *malloc(size_t bytes) {
  if (!heap_base) {
    heap_init(bytes);
  }
  if (bytes == 0) {
    return NULL;
  }
  // Start from first free, find first open segment that can hold 'bytes'
  heap_seg *ptr = first_free;
  heap_seg *best = NULL;

  while (ptr && ptr < heap_end) {
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
    size_t next_size = old_cap - segment_size(ptr);
    if (next_size <= sizeof(heap_seg)) {
      ptr->size += next_size;
      update_back(best, best->next_free, false);
    } else if (old_cap > ptr->size) {
      heap_seg *next_block = (void *)ptr + segment_size(ptr);
      *next_block =
          (heap_seg){old_cap - segment_size(ptr), ptr, ptr->next_free};
      heap_seg *next_next_block = (void *)next_block + segment_size(next_block);
      if (next_next_block < heap_end) {
        next_next_block->prev = next_block;
      }

      // Scan for previous free block, update links
      update_back(best, next_block, false);
    }
    return ptr + 1;
  }

  // Ran out of space
  heap_seg *prev = NULL;
  if (!ptr) {
    ptr = heap_base;
    while (ptr < heap_end) {
      prev = ptr;
      ptr = (void *)ptr + segment_size(ptr);
    }
    // We are writing outside of memory here,
    // need to find a way to break this, and track prev
    // without writing it outside of the heap
    if (ptr < heap_end) {
      ptr->prev = prev;
    }
  }
  size_t next_size;
  size_t increment = (bytes > HEAP_INC) ? round_bytes(bytes) : HEAP_INC;
  size_t new_size = round_bytes(bytes) | 0x1;

  heap_seg *new_seg = ptr;

  size_t old_size = (new_seg < heap_end) ? new_seg->size : 0;
  if (new_seg < heap_end && new_seg->prev && segment_free(new_seg->prev)) {
    // Previous chunk is free
    new_seg = new_seg->prev;
    if ((sbrk(increment)) == (void *)-1) {
      return NULL;
    }
    new_seg->size = new_size + sizeof(heap_seg);
    next_size = (old_size + increment) - segment_size(new_seg);
  } else {
    // Previous chunk is not free
    size_t cur_size = new_size + sizeof(heap_seg);
    if (round_bytes(bytes) < increment) {
      if ((sbrk(increment)) == (void *)-1) {
        return NULL;
      }
      next_size = old_size + increment - (cur_size ^ 0x1);
    } else {
      if ((sbrk(increment + sizeof(heap_seg))) == (void *)-1) {
        return NULL;
      }
      next_size =
          old_size + increment - (cur_size ^ 0x1) + sizeof(heap_seg);
    }
    new_seg->size = cur_size;
  }
  heap_end = sbrk(0);

  // Check if the next segment should be initialized
  if (next_size <= sizeof(heap_seg)) {
    ptr->size += next_size;
  } else if (next_size > 0) {
    heap_seg *next_block = (void *)new_seg + segment_size(new_seg);
    *next_block = (heap_seg){next_size, new_seg, NULL};

    assert((void *)next_block + segment_size(next_block) == heap_end);
    // Scan for previous free block, update links
    update_back(new_seg, next_block, false);
  }

  // Return user writable address
  return new_seg + 1;
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
    update_back(prev, header, true);
    header->next_free = next->next_free;
    header->size ^= 1;
    return;
  }
  update_back(prev, header, true);
  header->size ^= 1;
}

void *calloc(size_t count, size_t size) {
  void *ptr = malloc(count * size);
  if (!ptr) {
    return NULL;
  }
  memset(ptr, 0, count * size);
  return ptr;
}

void heap_walk();

void *realloc(void *ptr, size_t size) {
  size_t min = (size < segment_size((heap_seg *)(ptr - 1)))
                   ? size
                   : segment_size((heap_seg *)(ptr - 1)) - sizeof(heap_seg);
  free(ptr);
  if (size == 0) {
    return NULL;
  }
  heap_seg *new_ptr = malloc(size);
  if (!new_ptr) {
    return NULL;
  }
  memmove(new_ptr, ptr, min);
  return new_ptr;
}

/* void heap_size() { */
/*   printf("Heap is %ld bytes\n", ((void *)heap_end - (void *)heap_base)); */
/* } */

/* void heap_walk() { */
/*   size_t total_size = 0; */
/*   heap_seg *ptr = heap_base; */
/*   while (ptr < heap_end && ptr->size) { */
/*     printf("Segment size: %4ld, Prev: %14p, Value: %3d, Address: %p", */
/* ptr->size, */
/*            ptr->prev, (ptr->size & 1) ? *(int8_t *)(ptr + 1) : (int8_t)-1, */
/* ptr); */
/*     if (segment_free(ptr)) { */
/*       printf(" --> Next Free: %p\n", ptr->next_free); */
/*     } else { */
/*       printf("\n"); */
/*     } */
/*     total_size += segment_size(ptr); */
/*     ptr = (void *)ptr + segment_size(ptr); */
/*   } */
/*   printf("\nEnding Size: %ld\n", total_size); */
/* } */

/* int main() { */
/*   int8_t *block = malloc(242); */
/*   *block = 4; */

/*   int8_t *num = malloc(243); */
/*   *num = 7; */

/*   int8_t *test = malloc(244); */
/*   *test = 66; */

/*   int8_t *big_num = malloc(245); */
/*   *big_num = 64; */

/*   int8_t *buf = malloc(1024); */
/*   *buf = 7; */

/*   int8_t *small_num = calloc(64, 8); */
/*   *small_num = 99; */

/*   free(block); */

/*   void *reallocated = realloc(small_num, 8); */
/*   assert(reallocated); */

/*   int8_t *testing_clear = malloc(9000); */

/*   heap_walk(); */
/*   heap_size(); */
/*   printf("end: %p\n\n", heap_end); */

/*   return 0; */
/* } */
