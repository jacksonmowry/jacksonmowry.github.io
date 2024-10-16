#include "pagealloc.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TOTAL_PAGES 100

int main() {
  assert(page_init(262144));

  void *allocations[100];
  int size[100];
  int num_allocations = 0;
  int allocated_size = 0;

  for (int i = 0; i < 100; i++) {
    int allocation_size = rand() % 10 + 1;
    size[i] = allocation_size;

    void *allocation = page_alloc(allocation_size);
    assert(allocation != NULL);

    allocations[num_allocations++] = allocation;

    allocated_size += allocation_size;
  }

  assert(pages_taken() == allocated_size);
  assert(pages_free() == 262144 - allocated_size);

  for (int i = 0; i < 100; i += 2) {
    allocated_size -= size[i];
    page_free(allocations[i]);
  }

  assert(pages_taken() == allocated_size);
  assert(pages_free() == 262144 - allocated_size);

  for (int i = 0; i < 100; i += 2) {
    int allocation_size = rand() % 10 + 1;
    size[i] = allocation_size;

    void *allocation = page_alloc(allocation_size);
    assert(allocation != NULL);

    allocations[i] = allocation;

    allocated_size += allocation_size;
  }

  assert(pages_taken() == allocated_size);
  assert(pages_free() == 262144 - allocated_size);
  printf("pages in use: %zu\n", pages_taken());

  assert(!page_alloc(999999));

  for (int i = 0; i < 100; i++) {
    page_free(allocations[i]);
  }

  assert(pages_taken() == 0);
  assert(pages_free() == 262144);

  page_free(NULL);
  page_free((void *)0xFFFFFFFFFFFFFFFF);

  page_deinit();

  page_init(100);

  void *one = page_alloc(10);
  void *two = page_alloc(10);
  void *three = page_alloc(10);

  page_free(two);

  void *four = page_alloc(5);
  void *five = page_alloc(5);

  assert(four < three);
  assert(five < three);

  page_free(three);
  void *six = page_alloc(1);

  assert(three == six);
}
