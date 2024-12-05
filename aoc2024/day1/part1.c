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
  void vector_##type##_free(vector_##type v);

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
  void vector_##type##_free(vector_##type v) { free(v.array); }

VECTOR_PROTOTYPE(int);
VECTOR(int);

static int int_comp(const void *p1, const void *p2) {
  int left = *(int *)p1;
  int right = *(int *)p2;

  return left - right;
}

int main() {
  vector_int left = vector_int_init(10);
  vector_int right = vector_int_init(10);

  int a, b;

  while (scanf("%d %d", &a, &b) == 2) {
    vector_int_pb(&left, a);
    vector_int_pb(&right, b);
  }

  qsort(left.array, left.len, sizeof(*left.array), int_comp);
  qsort(right.array, right.len, sizeof(*right.array), int_comp);

  int accumulator = 0;

  for (int i = 0; i < left.len; i++) {
    accumulator += abs(right.array[i] - left.array[i]);
  }

  printf("%d\n", accumulator);
}
