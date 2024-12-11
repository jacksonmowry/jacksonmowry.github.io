#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>

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
  void vector_##type##_resize(vector_##type *v, size_t new_size);              \
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
  void vector_##type##_resize(vector_##type *v, size_t new_size) {             \
    v->len = new_size;                                                         \
    v->cap = new_size;                                                         \
    v->array = realloc(v->array, sizeof(type) * new_size);                     \
  }                                                                            \
  void vector_##type##_free(vector_##type *v) { free(v->array); }

VECTOR_PROTOTYPE(int);
VECTOR(int);

int main() {
  __attribute__((cleanup(vector_int_free))) vector_int left =
      vector_int_init(10);
  __attribute__((cleanup(vector_int_free))) vector_int right =
      vector_int_init(1);
  memset(right.array, 0, sizeof(int) * 1);

  int a, b;

  while (scanf("%d %d", &a, &b) == 2) {
    vector_int_pb(&left, a);

    if (b >= right.len) {
      size_t prev_index = right.len;
      vector_int_resize(&right, b + 1);
      memset(right.array + prev_index, 0,
             (right.len - prev_index) * sizeof(int));
    }

    right.array[b]++;
  }

  int accumulator = 0;

  for (int i = 0; i < left.len; i++) {
    if (left.array[i] >= right.len) {
      continue;
    }
    accumulator += left.array[i] * right.array[left.array[i]];
  }

  printf("%d\n", accumulator);
}
