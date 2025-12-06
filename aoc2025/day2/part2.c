#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  long start, stop;
  long sum = 0;

  while (2 == (scanf("%ld-%ld", &start, &stop))) {
    for (long lower = start; lower <= stop; lower++) {

      char buf[256];
      long len = sprintf(buf, "%ld", lower);

      long len_2 = len / 2;
      bool valid = false;
      if (lower == 111) {
        printf("Testing 111\n");
      }
      for (int window_size = 1; window_size <= len_2; window_size++) {
        if (lower == 111) {
          printf("Window size of %d\n", window_size);
        }
        if (len % window_size != 0) {
          continue;
        }

        char window_chars[window_size];
        for (int i = 0; i < window_size; i++) {
          window_chars[i] = buf[i];
        }

        for (int i = window_size; i < len; i += window_size) {
          for (int j = 0; j < window_size; j++) {
            if (window_chars[j] != buf[i + j]) {
              goto WINDOW_LOOP;
            }
          }
        }
        valid = true;
        break;
      WINDOW_LOOP:;
      }

      if (valid) {
        printf("Invalid id: %s\n", buf);
        sum += lower;
      }
    }

    char c;
    if (1 != (scanf("%c", &c)) || c != ',') {
      break;
    }
  }

  printf("Sum: %ld\n", sum);
}
