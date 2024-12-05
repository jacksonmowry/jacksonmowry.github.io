#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*********************************/
/* generic vector implementation */
/*********************************/
#define VECTOR_PROTOTYPE(type)                                                 \
  typedef struct vector_##type {                                               \
    type *array;                                                               \
    size_t len;                                                                \
    size_t cap;                                                                \
  } vector_##type;                                                             \
  void vector_##type##_pb(vector_##type *v, type item);                        \
  type vector_##type##_get(vector_##type *v, size_t index);                    \
  vector_##type vector_##type##_init(size_t cap);                              \
  vector_##type vector_##type##_from(const type *const input, size_t size);    \
  bool vector_##type##_eq(vector_##type a, vector_##type b,                    \
                          bool (*equality_func)(void *, void *));              \
  bool vector_##type##_cmp(vector_##type a, vector_##type b);                  \
  void vector_##type##_free(vector_##type *v);

#define VECTOR(type)                                                           \
  void vector_##type##_pb(vector_##type *v, type item) {                       \
    if (!v) {                                                                  \
      fprintf(stderr, "Attempting to call pb on a NULL vector\n");             \
      exit(1);                                                                 \
    }                                                                          \
    if (!v->array) {                                                           \
      fprintf(stderr, "Attempting to call pb on a vector with NULL array\n");  \
      exit(1);                                                                 \
    }                                                                          \
    if (v->len >= v->cap) {                                                    \
      v->cap *= 2;                                                             \
      v->array = (type *)realloc(v->array, v->cap * sizeof(type));             \
    }                                                                          \
    v->array[v->len++] = item;                                                 \
  }                                                                            \
  type vector_##type##_get(vector_##type *v, size_t index) {                   \
    if (index >= v->len) {                                                     \
      fprintf(stderr, "Attempt to access memory oob for v[%zu], index %zu\n",  \
              v->len, index);                                                  \
      exit(1);                                                                 \
    }                                                                          \
    return v->array[index];                                                    \
  }                                                                            \
  vector_##type vector_##type##_init(size_t cap) {                             \
    return (vector_##type){.array = (type *)calloc(cap, sizeof(type)),         \
                           .cap = cap};                                        \
  }                                                                            \
  vector_##type vector_##type##_from(const type *const input, size_t size) {   \
    vector_##type blank = vector_##type##_init(size);                          \
    if (!blank.array) {                                                        \
      fprintf(stderr, "Failed to allocate vector_##type##_init\n");            \
      exit(1);                                                                 \
    }                                                                          \
    blank.len = size;                                                          \
    memcpy(blank.array, input, sizeof(type) * size);                           \
    return blank;                                                              \
  }                                                                            \
  bool vector_##type##_eq(vector_##type a, vector_##type b,                    \
                          bool (*equality_func)(void *, void *)) {             \
    if (a.len != b.len) {                                                      \
      return false;                                                            \
    }                                                                          \
    for (size_t i = 0; i < a.len; i++) {                                       \
      if (!equality_func(&a.array[i], &b.array[i])) {                          \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
    return true;                                                               \
  }                                                                            \
  bool vector_##type##_cmp(vector_##type a, vector_##type b) {                 \
    if (a.len != b.len) {                                                      \
      return false;                                                            \
    }                                                                          \
    return !memcmp(a.array, b.array, sizeof(type) * a.len);                    \
  }                                                                            \
  void vector_##type##_free(vector_##type *v) { free(v->array); }

typedef enum Token_Type {
  lparen,
  rparen,
  comma,
  whitespace,
  mul,
  dont_mul,
  do_mul,
  digit,
  junk
} Token_Type_t;

typedef struct Token {
  Token_Type_t type;
  char *content;
  size_t len;
} Token_t;

char *Token_Type_str[] = {"lparen", "rparen", "comma", "whitespace", "mul",
                          "don't",  "do",     "digit", "junk"};

VECTOR_PROTOTYPE(Token_t);
VECTOR(Token_t);

int main() {
  bool mul_enabled = true;
  char buf[4000];
  int accum = 0;
  while (fgets(buf, sizeof(buf), stdin)) {

    __attribute__((cleanup(vector_Token_t_free))) vector_Token_t v =
        vector_Token_t_init(10);

    size_t cursor = 0;
    while (cursor < strlen(buf)) {
      switch (buf[cursor]) {
      case '(':
        vector_Token_t_pb(
            &v, (Token_t){.type = lparen, .content = buf + cursor, .len = 1});
        cursor++;
        break;

      case ')':
        vector_Token_t_pb(
            &v, (Token_t){.type = rparen, .content = buf + cursor, .len = 1});
        cursor++;
        break;

      case ',':
        vector_Token_t_pb(
            &v, (Token_t){.type = comma, .content = buf + cursor, .len = 1});
        cursor++;
        break;

      case '\n':
      case ' ':
      case '\t':
        vector_Token_t_pb(
            &v,
            (Token_t){.type = whitespace, .content = buf + cursor, .len = 1});
        cursor++;
        break;

      case 'm':
        if (!strncmp("mul", buf + cursor, 3)) {
          vector_Token_t_pb(
              &v, (Token_t){.type = mul, .content = buf + cursor, .len = 3});
          cursor += 3;
        } else {
          vector_Token_t_pb(
              &v, (Token_t){.type = junk, .content = buf + cursor, .len = 1});
          cursor++;
        }
        break;

      case 'd':
        if (!strncmp("don't", buf + cursor, 5)) {
          vector_Token_t_pb(
              &v,
              (Token_t){.type = dont_mul, .content = buf + cursor, .len = 5});
          cursor += 5;
        } else if (!strncmp("do", buf + cursor, 2)) {
          vector_Token_t_pb(
              &v, (Token_t){.type = do_mul, .content = buf + cursor, .len = 5});
          cursor += 2;
        }

      default:
        if (isdigit(buf[cursor])) {
          size_t tmp_len = 0;

          while (isdigit(buf[cursor + tmp_len])) {
            tmp_len++;
          }

          vector_Token_t_pb(&v, (Token_t){.type = digit,
                                          .content = buf + cursor,
                                          .len = tmp_len});
          cursor += tmp_len;
        } else {
          vector_Token_t_pb(
              &v, (Token_t){.type = junk, .content = buf + cursor, .len = 1});
          cursor++;
        }
      }
    }

    for (size_t i = 0; i < v.len; i++) {
      if (v.array[i].type != whitespace) {
        printf("%s: %.*s\n", Token_Type_str[v.array[i].type],
               (int)v.array[i].len, v.array[i].content);
      } else {
        printf("%s: ' '\n", "whitespace");
      }
    }

    for (size_t i = 0; i < v.len - 6; i++) {
      if (mul_enabled) {
        if (v.array[i].type == dont_mul) {
          mul_enabled = false;
          continue;
        }
        if (v.array[i].type == mul && v.array[i + 1].type == lparen &&
            v.array[i + 2].type == digit && v.array[i + 3].type == comma &&
            v.array[i + 4].type == digit && v.array[i + 5].type == rparen) {
          // Valid state, parse digits and mult
          int digit_a = 0, digit_b = 0;
          sscanf(v.array[i + 2].content, "%d", &digit_a);
          sscanf(v.array[i + 4].content, "%d", &digit_b);

          if (digit_a > 999 || digit_b > 999) {
            i++;
            continue;
          }

          printf("%d * %d = %d\n", digit_a, digit_b, digit_a * digit_b);
          accum += digit_a * digit_b;
        }
      } else {
        if (v.array[i].type == do_mul) {
          mul_enabled = true;
        }
      }
    }
  }

  printf("%d\n", accum);
}
