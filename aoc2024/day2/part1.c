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

    printf("len: %d\t", arr_len);

    if (arr_len == 1) {
      break;
    }

    for (int i = 0; i < arr_len; i++) {
      printf("%d ", arr[i]);
    }

    bool increasing = arr[1] > arr[0];

    for (int i = 1; i < arr_len; i++) {
      if (arr[i] > arr[i - 1] != increasing) {
        printf(" unsafe\n\n");
        goto end;
      }

      printf("%d ", arr[i] - arr[i - 1]);

      if (arr[i] == arr[i - 1]) {
        printf(" unsafe\n\n");
        goto end;
      }

      int ab = abs(arr[i] - arr[i - 1]);

      if (ab < 1 || ab > 3) {
        printf(" unsafe\n\n");
        goto end;
      }
    }

    printf(" safe\n\n");
    count++;

  end:
    arr_len = 0;
    continue;
  }

  printf("%d\n", count);
}
