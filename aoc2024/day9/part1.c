#include <stdint.h>
#include <stdio.h>

int main() {
  int16_t disk[500000];
  size_t disk_len = 0;
  size_t blocks = 0;
  size_t used = 0;

  char tmp;
  while (scanf("%c", &tmp) == 1) {
    int16_t num = tmp - '0';

    if (blocks % 2 == 0) {
      while (num > 0) {
        disk[disk_len++] = blocks / 2;
        num--;
        used++;
      }
    } else {
      while (num > 0) {
        disk[disk_len++] = -1;
        num--;
      }
    }

    blocks++;
  }

  size_t tail_cursor = disk_len - 1;
  size_t checksum = 0;

  for (int i = 0; i < used; i++) {
    if (disk[i] == -1) {
      while (disk[tail_cursor] == -1) {
        tail_cursor--;
      }

      disk[i] = disk[tail_cursor];
      disk[tail_cursor] = -1;
    } else {
      /* printf("%d", disk[i]); */
    }

    checksum += disk[i] * i;
  }

  printf("%zu\n", checksum);
}
