#include <stdbool.h>
#include <stdio.h>

bool is_valid(int i) {
  char num[7];
  sprintf(num, "%d", i);
  bool adjacent_sat = false;
  int adjacent_run = 1;

  for (int j = 0; j < 5; j++) {
    if (num[j] > num[j + 1]) {
      return false;
    }
    if (num[j] == num[j + 1]) {
      adjacent_run++;
    } else {
      if (adjacent_run == 2) {
        adjacent_sat = true;
      }
      adjacent_run = 1;
    }
  }
  if (adjacent_run == 2) {
    adjacent_sat = true;
  }

  if (adjacent_sat) {
    printf("%d\n", i);
  }

  return adjacent_sat;
}

int main() {
  int lower, upper;

  scanf("%d-%d", &lower, &upper);

  int total = 0;
  for (int i = lower; i <= upper; i++) {
    total += is_valid(i);
  }

  printf("%d\n", total);
}
