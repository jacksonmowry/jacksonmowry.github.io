#include "pagealloc.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
int main() {
  page_init(16385);
  void *a = page_alloc(16384);

  *(uint8_t *)a = 5;

  /* page_free(a); */

  /* a = page_alloc(3); */

  printf("%d\n", *(uint8_t *)a);

  uint8_t *b = page_alloc(1);
  *b = 42;
  printf("%d\n", *b);

  /* uint8_t *c = page_alloc(1); */
  /* assert(!c); */

  b[4095] = 1;

  printf("%d\n", b[4095]);

  printf("pages in use %zu\n", pages_taken());

  page_deinit();
}
