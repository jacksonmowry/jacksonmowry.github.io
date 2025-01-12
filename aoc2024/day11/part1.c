#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  uint64_t *input = malloc(sizeof(uint64_t) * 1000000000);
  size_t input_len = 0;

  while (scanf("%lu", &input[input_len++]) == 1) {
    ;
  }
  input_len--;

  /* for (int i = 0; i < input_len; i++) { */
  /*   printf("%lu ", input[i]); */
  /* } */
  /* putchar('\n'); */

  for (int i = 0; i < 75; i++) {
    printf("%d\n", i);
    int64_t *new = malloc(sizeof(uint64_t) * 1000000000);
    size_t new_len = 0;

    for (int j = 0; j < input_len; j++) {
      char buf[256];
      size_t buf_len = 0;

      if (input[j] == 0) {
        new[new_len++] = 1;
        continue;
      } else if (sprintf(buf, "%lu", input[j]) && strlen(buf) % 2 == 0) {
        size_t half_len = strlen(buf) / 2;

        memmove(buf + half_len + 1, buf + half_len, (strlen(buf) / 2) + 1);
        buf[half_len] = ' ';
        /* printf("split %d into buf: %s\n", input[j], buf); */

        sscanf(buf, "%lu", &new[new_len++]);
        sscanf(buf + half_len + 1, "%lu", &new[new_len++]);

        /* printf("split formed %d %d\n", new[new_len - 2], new[new_len - 1]);
         */
        continue;
      } else {
        new[new_len++] = input[j] * 2024;
        continue;
      }
    }

    /* printf("After %d blinks:\n", i + 1); */
    /* for (int k = 0; k < new_len; k++) { */
    /*   printf("%lu ", new[k]); */
    /* } */
    /* putchar('\n'); */

    memcpy(input, new, new_len * sizeof(int64_t));
    input_len = new_len;
    free(new);
  }

  printf("%zu\n", input_len);
}
