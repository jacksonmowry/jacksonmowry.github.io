#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

typedef struct Page {
  union {
    char c[4096];
    int i[4096 / 4];
    long l[4096 / 8];
  } u;
} Page;

int main() {

  void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap");
    return -1;
  }

  Page *p = (Page *)ptr;
  p->u.i[0] = 100;

  printf("%d\n", p->u.i[0]);

  munmap(ptr, 4096);
}
