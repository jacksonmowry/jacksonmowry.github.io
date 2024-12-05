#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  int arr[10];
  int arr_len = 0;
  int count = 0;

  char buf[256] = {0};

  while (!feof(stdin)) {
    while (scanf("%d", arr + arr_len++) == 1) {
      char thing;
      if ((thing = fgetc(stdin)) == '\n') {
        break;
      }

      ungetc(thing, stdin);
    }

    if (arr_len == 1) {
      break;
    }

    bool increasing = arr[1] > arr[0];
    int cur_idx = 1;
    int prev_idx = 0;
    int idx_to_skip = -1;
    int problems_resolved = 0;

    // Essentially try running through and if we make it to the end still
    // invalid we can remove the first index

    for (int i = 0; i < arr_len; i++) {
      printf("%d ", arr[i]);
    }

    for (int i = 1; i < arr_len; i++) {
      if (prev_idx == idx_to_skip) {
        prev_idx++;
      }
      if (arr[i] > arr[prev_idx] != increasing) {
        problems_resolved++;
        idx_to_skip = i;
        if (problems_resolved <= 1) {
          continue;
        } else {
          printf("unsafe\n");
          goto end;
        }
      }

      if (arr[i] == arr[prev_idx]) {
        problems_resolved++;
        idx_to_skip = i;
        if (problems_resolved <= 1) {
          continue;
        } else {
          printf("unsafe\n");
          goto end;
        }
      }

      int ab = abs(arr[i] - arr[prev_idx]);

      if (ab < 1 || ab > 3) {
        problems_resolved++;
        idx_to_skip = i;
        if (problems_resolved <= 1) {
          continue;
        } else {
          printf("unsafe\n");
          goto end;
        }
      }

      prev_idx++;
    }

    printf("safe: idx_skipped: %d\n", idx_to_skip);
    count++;
    arr_len = 0;
    continue;

  end:
    increasing = arr[2] > arr[1];
    for (int i = 2; i < arr_len; i++) {
      if (arr[i] > arr[i - 1] != increasing) {
        goto for_real;
      }

      if (arr[i] == arr[i - 1]) {
        goto for_real;
      }

      int ab = abs(arr[i] - arr[i - 1]);

      if (ab < 1 || ab > 3) {
        goto for_real;
      }
    }

    count++;

  for_real:
    arr_len = 0;
    continue;
  }

  printf("%d\n", count);
}
