#include <assert.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HEAP_INC 1024

typedef struct heap_seg {
  size_t size;
  struct heap_seg *prev;
  struct heap_seg *next_free;
} heap_seg;

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
    } else if (old_cap > ptr->size) {
      heap_seg *next_block = (void *)ptr + segment_size(ptr);
      *next_block =
          (heap_seg){old_cap - segment_size(ptr), ptr, ptr->next_free};
      heap_seg *next_next_block = (void *)next_block + segment_size(next_block);
      next_next_block->prev = next_block;

      // Scan for previous free block, update links
      update_back(best, next_block, false);
    }
    return ptr + 1;
  }

  // Ran out of space
  size_t next_size;
  size_t increment = (bytes > HEAP_INC) ? round_bytes(bytes) : HEAP_INC;
  size_t new_size = round_bytes(bytes) | 0x1;

  heap_seg *new_seg = (void *)ptr + segment_size(ptr);
  *new_seg = (heap_seg){0, ptr, NULL};

  if (new_seg->prev && segment_free(new_seg->prev)) {
    // Previous chunk is free
    new_seg = new_seg->prev;
    if ((sbrk(increment)) == (void *)-1) {
      return NULL;
    }
    size_t old_size = new_seg->size;
    new_seg->size = new_size + sizeof(heap_seg);
    next_size = (old_size + increment) - segment_size(new_seg);
  } else {
    // Previous chunk is not free
    if ((sbrk(increment + sizeof(heap_seg))) == (void *)-1) {
      return NULL;
    }
    new_seg->size = new_size + sizeof(heap_seg);
    next_size = increment - segment_size(new_seg);
  }
  heap_end = sbrk(0);

  // Check if the next segment should be initialized
  assert(next_size > 0);
  if (next_size <= sizeof(heap_seg)) {
    ptr->size += next_size;
  } else if (next_size > 0) {
    heap_seg *next_block = (void *)ptr + segment_size(ptr);
    *next_block = (heap_seg){next_size, ptr, NULL};

    // Scan for previous free block, update links
    update_back(new_seg, next_block, false);
  }

  // Return user writable address
  return ptr + 1;
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

  update_back(prev, header, true);
}

void *calloc(size_t count, size_t size) {
  void *ptr = malloc(count * size);
  if (!ptr) {
    return NULL;
  }
  memset(ptr, 0, count * size);
  return ptr;
}

void *realloc(void *ptr, size_t size) {
	size_t min = (size < segment_size((heap_seg*)(ptr-1))) ? size : segment_size((heap_seg*)(ptr-1));
	free(ptr);
	if (size == 0) {
		return NULL;
	}
	heap_seg *new_ptr = malloc(size);
	if (!new_ptr) {
		return NULL;
	}
	memmove(new_ptr+1, ptr, min);
	return new_ptr+1;
}

void heap_size() {
  printf("Heap is %ld bytes\n", (heap_end - heap_base) * sizeof(heap_seg));
}

void heap_walk() {
  heap_seg *ptr = heap_base;
  while (ptr < (heap_seg *)heap_end) {
    printf("Segment size: %4ld, Prev: %14p, Value: %3d, Address: %p", ptr->size,
           ptr->prev, (ptr->size & 1) ? *(int8_t *)(ptr + 1) : (int8_t)-1, ptr);
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

  free(block);
  heap_walk();
  return 0;
}
