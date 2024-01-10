#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Helper macro for counting the number of arguments
#define NUM_ARGS(...)                                                          \
  NUM_ARGS_IMPL(__VA_ARGS__, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, \
                117, 116, 115, 114, 113, 112, 111, 110, 109, 108, 107, 106,    \
                105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92,  \
                91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77,    \
                76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62,    \
                61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47,    \
                46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,    \
                31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,    \
                16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// Helper macro for expanding the arguments
#define NUM_ARGS_IMPL(                                                         \
    _127, _126, _125, _124, _123, _122, _121, _120, _119, _118, _117, _116,    \
    _115, _114, _113, _112, _111, _110, _109, _108, _107, _106, _105, _104,    \
    _103, _102, _101, _100, _99, _98, _97, _96, _95, _94, _93, _92, _91, _90,  \
    _89, _88, _87, _86, _85, _84, _83, _82, _81, _80, _79, _78, _77, _76, _75, \
    _74, _73, _72, _71, _70, _69, _68, _67, _66, _65, _64, _63, _62, _61, _60, \
    _59, _58, _57, _56, _55, _54, _53, _52, _51, _50, _49, _48, _47, _46, _45, \
    _44, _43, _42, _41, _40, _39, _38, _37, _36, _35, _34, _33, _32, _31, _30, \
    _29, _28, _27, _26, _25, _24, _23, _22, _21, _20, _19, _18, _17, _16, _15, \
    _14, _13, _12, _11, _10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...)       \
  N

#define println(...) println_impl(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

#define $(var)                                                                 \
  _Generic((var),                                                              \
      int8_t: auto_asprintf("%d", var),                                        \
      int16_t: auto_asprintf("%d", var),                                       \
      int32_t: auto_asprintf("%d", var),                                       \
      int64_t: auto_asprintf("%ld", var),                                      \
      uint8_t: auto_asprintf("%u", var),                                       \
      uint16_t: auto_asprintf("%u", var),                                      \
      uint32_t: auto_asprintf("%u", var),                                      \
      uint64_t: auto_asprintf("%lu", var),                                     \
      float: auto_asprintf("%f", var),                                         \
      double: auto_asprintf("%f", var),                                        \
      char *: auto_asprintf("%s", var),                                        \
      const char *: auto_asprintf("%s", var))

char *auto_asprintf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  char *str = NULL;
  int length = vasprintf(&str, format, args);

  va_end(args);

  if (length == -1) {
    fprintf(stderr, "Failed to allocate memory\n");
    return NULL;
  }

  return str;
}

void println_impl(int num_args, ...) {
  char buf[4096] = {0};
  char *pos = buf;
  va_list args;
  va_start(args, num_args);

  for (int i = 0; i < num_args; i++) {
    char *str_val = va_arg(args, char *);
    pos = stpcpy(pos, str_val);
    free(str_val);
  }
  va_end(args);
  *pos++ = '\n';
  *pos = '\0';
  ssize_t bytes_written = write(STDOUT_FILENO, buf, pos - buf);
  if (bytes_written == -1) {
    perror("write");
    exit(1);
  }
}

int main() {
  int a = 5;
  println($(UINT64_MAX), $(" "), $(INT64_MAX), $(" hi "), $(3.14f), $(" "),
          $(a));
}
