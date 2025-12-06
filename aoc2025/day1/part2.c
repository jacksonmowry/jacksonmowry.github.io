#include <stdio.h>
#include <stdlib.h>

int main() {
  int pos = 50;
  int count = 0;

  char buf[256];
  while (fgets(buf, sizeof(buf), stdin)) {
    char direction = buf[0];
    char *distance = buf + 1;
    int d;
    sscanf(distance, "%d", &d);

    for (int i = 0; i < d; i++) {
      pos = (pos + (direction == 'L' ? -1 : 1)) % 100;

      if (pos == 0) {
        count++;
      }
    }
  }

  printf("Password: %d\n", count);
}
