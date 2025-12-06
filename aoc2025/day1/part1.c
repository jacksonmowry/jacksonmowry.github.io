#include <stdio.h>

int main() {
  int pos = 50;
  int count = 0;

  char buf[256];
  while (fgets(buf, sizeof(buf), stdin)) {
    char direction = buf[0];
    char *distance = buf + 1;
    int d;
    sscanf(distance, "%d", &d);

    pos = (pos + (direction == 'L' ? -d : d)) % 100;

    if (pos == 0) {
      count++;
    }
  }

  printf("Password: %d\n", count);
}
