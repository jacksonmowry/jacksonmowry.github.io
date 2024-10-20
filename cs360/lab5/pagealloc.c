// Jackson Mowry
// Lab5
// Page Allocator
// Wed Oct 16 07:21:13 2024
#include "pagealloc.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096

// 3 globals to help keep the state of the page allocator
static void *pages_ptr;
static size_t num_pages;
static size_t without_bk;

/*******************************************************/
/* The following 6 functions help us to test/set/clear */
/* the bits for a page/end in bookkeeping              */
/*******************************************************/

bool test_page(uint8_t *book, int page) {
  return book[page / 4] >> (7 - (page % 4 * 2)) & 1;
}

bool test_end(uint8_t *book, int page) {
  return book[page / 4] >> (6 - (page % 4 * 2)) & 1;
}

void set_page(uint8_t *book, int page) {
  book[page / 4] |= 1 << (7 - (page % 4 * 2));
}

void set_end(uint8_t *book, int page) {
  book[page / 4] |= 1 << (6 - (page % 4 * 2));
}

void clear_page(uint8_t *book, int page) {
  book[page / 4] &= ~(1 << (7 - (page % 4 * 2)));
}

void clear_end(uint8_t *book, int page) {
  book[page / 4] &= ~(1 << (6 - (page % 4 * 2)));
}

// Allocator functions

// Allocates the entire pool of memory which is given in pages. Pages for
// bookkeeping are added automatically.
bool page_init(size_t pages) {
  // Bounds checking
  if (pages < 2 || pages > (1 << 18)) {
    return false;
  }

  // Rounding up to a page bound
  size_t bookkeeping_pages = (pages + 16383) / 16384;

  pages_ptr = mmap(NULL, (pages + bookkeeping_pages) * PAGE_SIZE,
                   PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  // Check for mmap failure
  if (pages_ptr == MAP_FAILED) {
    return false;
  }

  // 2 numbers are easier than 1 number
  num_pages = pages + bookkeeping_pages;
  without_bk = pages;

  return true;
}

// Deallocates the entire pool of memory allocated by the above function.
void page_deinit(void) {
  // Don't do anything if we didn't do anything am I right
  if (!pages_ptr) {
    return;
  }

  munmap(pages_ptr, num_pages * PAGE_SIZE);
  pages_ptr = NULL;
  num_pages = 0;
  without_bk = 0;
}

// Allocation functions

// Returns a contiguous allocation of pages. Error checking is in place to not
// allow an invalid parameter of pages to be passed.
void *page_alloc(size_t pages) {
  // Our error checking
  if (!pages_ptr) {
    return NULL;
  }

  if (pages == 0) {
    return NULL;
  }

  if (pages_free() < pages) {
    return NULL;
  }

  if (pages > without_bk) {
    return NULL;
  }

  void *start_of_pages = (pages_ptr + ((num_pages - without_bk) * PAGE_SIZE));

  // Find a possible starting spot, then find the number of pages we actually
  // need
  for (int i = 0; i < without_bk; i++) {
    bool found = true;

    for (int j = i; j < (i + pages); j++) {
      if (test_page(pages_ptr, j)) {
        found = false;
        break;
      }
    }

    if (found) {
      // We found a spot, now mark it as allocated, including the end bit
      for (int j = i; j < (i + pages); j++) {
        set_page(pages_ptr, j);
      }
      set_end(pages_ptr, i + pages - 1);

      return (uint8_t *)start_of_pages + (i * PAGE_SIZE);
    }
  }

  // If we make it here we have space, but it coudn't fulfil the allocation
  // request
  return NULL;
}

// Takes in the starting memory address of an allocation and frees all
// associated pages in the allocation.
// Does not attempt to free an invalid address.
void page_free(void *addr) {
  // Don't do anything if we haven't initialized
  if (!pages_ptr) {
    return;
  }

  // This address is outside the bounds we allocated
  if (addr < pages_ptr ||
      (uint8_t *)addr > (uint8_t *)pages_ptr + (PAGE_SIZE * num_pages)) {
    return;
  }

  uint8_t *start_of_pages =
      (pages_ptr + ((num_pages - without_bk) * PAGE_SIZE));

  // Pointer arithmetic to calculate which page this is
  size_t page_num = ((uint8_t *)addr - start_of_pages) / PAGE_SIZE;

  // Clear everything until the end of the allocation
  while (!test_end(pages_ptr, page_num)) {
    clear_page(pages_ptr, page_num);
    page_num++;
  }

  // Finally clear the end bit
  clear_page(pages_ptr, page_num);
  clear_end(pages_ptr, page_num);
}

// Status functions
size_t pages_taken(void) {
  // Count up the page bits
  size_t count = 0;
  for (int i = 0; i < without_bk; i++) {
    count += test_page(pages_ptr, i);
  }

  return count;
}

size_t pages_free(void) {
  size_t count = 0;
  // Count up the opposite of the page pits
  for (int i = 0; i < without_bk; i++) {
    count += !test_page(pages_ptr, i);
  }

  return count;
}
