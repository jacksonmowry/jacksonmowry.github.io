#include <stdio.h>
#include <stdlib.h>

int main() {
  long start, stop;
  long sum = 0;

  while (2 == (scanf("%ld-%ld", &start, &stop))) {
    printf("I scanned %ld %ld\n", start, stop);
    for (long lower = start; lower <= stop; lower++) {

      char buf[256];
      long len = sprintf(buf, "%ld", lower);

      if (len % 2 != 0) {
        continue;
      }

      long len_2 = len / 2;
      for (long i = 0; i < len_2; i++) {
        if (buf[i] != buf[len_2 + i]) {
          goto END;
        }
      }

      printf("Invalid id: %s\n", buf);
      sum += lower;
    END:;
    }

    char c;
    if (1 != (scanf("%c", &c)) || c != ',') {
      break;
    }
  }

  printf("Sum: %ld\n", sum);
}
