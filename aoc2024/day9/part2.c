#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
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
  void vector_##type##_insert(vector_##type *v, type item, size_t index);      \
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
  void vector_##type##_insert(vector_##type *v, type item, size_t index) {     \
    if (!v) {                                                                  \
      fprintf(stderr, "Attempting to call pb on a NULL vector\n");             \
      exit(1);                                                                 \
    }                                                                          \
    if (!v->array) {                                                           \
      fprintf(stderr, "Attempting to call pb on a vector with NULL array\n");  \
      exit(1);                                                                 \
    }                                                                          \
    if (v->len + 1 >= v->cap) {                                                \
      v->cap *= 2;                                                             \
      v->array = (type *)realloc(v->array, v->cap * sizeof(type));             \
    }                                                                          \
    if (index >= v->len) {                                                     \
      exit(1);                                                                 \
    }                                                                          \
    memmove(v->array + index + 1, v->array + index,                            \
            (v->len - index) * sizeof(type));                                  \
    v->len++;                                                                  \
    v->array[index] = item;                                                    \
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

typedef struct Pair {
  int16_t id;
  size_t len;
  bool free_space;
} Pair_t;

VECTOR_PROTOTYPE(Pair_t);
VECTOR(Pair_t);

VECTOR_PROTOTYPE(int);
VECTOR(int);

int main() {
  vector_Pair_t v = vector_Pair_t_init(10);
  int16_t disk[500000];
  size_t disk_len = 0;
  size_t blocks = 0;
  size_t used = 0;

  char tmp;
  while (scanf("%c", &tmp) == 1) {
    if (tmp < '0' || tmp > '9') {
      break;
    }
    int16_t num = tmp - '0';

    if (blocks % 2 == 0) {
      vector_Pair_t_pb(
          &v, (Pair_t){.id = blocks / 2, .len = num, .free_space = false});
    } else {
      vector_Pair_t_pb(&v, (Pair_t){.id = -1, .len = num, .free_space = true});
    }

    blocks++;
  }

  /* for (int i = 0; i < v.len; i++) { */
  /*   /\* printf("{id: %hd, len: %zu, free_space: %s}\n", v.array[i].id, *\/ */
  /*   /\*        v.array[i].len, v.array[i].free_space ? "true" : "false"); *\/
   */

  /*   for (int j = 0; j < v.array[i].len; j++) { */
  /*     if (v.array[i].free_space) { */
  /*       putchar('.'); */
  /*     } else { */
  /*       printf("%d", v.array[i].id); */
  /*     } */
  /*   } */
  /* } */

  /* putchar('\n'); */

  for (int i = v.len - 1; i >= 0; i--) {
    if (v.array[i].free_space || v.array[i].id == -1) {
      continue;
    }

    for (int j = 0; j < i; j++) {
      if (!v.array[j].free_space) {
        continue;
      }

      if (v.array[i].len <= v.array[j].len) {
        /* printf("swapping\n {%hd, %zu, %s} : {%hd, %zu, %s}\n", v.array[j].id,
         */
        /*        v.array[j].len, v.array[j].free_space ? "true" : "false", */
        /*        v.array[i].id, v.array[i].len, */
        /*        v.array[i].free_space ? "true" : "false"); */
        // Founda hole
        size_t remaining_space = v.array[j].len - v.array[i].len;
        v.array[j].len = v.array[i].len;
        v.array[j].free_space = false;
        v.array[j].id = v.array[i].id;
        assert(v.array[j].id != -1);

        v.array[i].id = -1;
        v.array[i].free_space = true;

        if (remaining_space) {
          vector_Pair_t_insert(
              &v,
              (Pair_t){.free_space = true, .id = -1, .len = remaining_space},
              j + 1);
        }
        break;
      }
    }

    for (int i = 0; i < v.len; i++) {
      if (v.array[i].len == 0) {
        continue;
      }
      /* for (int j = 0; j < v.array[i].len; j++) { */
      /*   if (v.array[i].free_space) { */
      /*     putchar('.'); */
      /*   } else { */
      /*     printf("%d", v.array[i].id); */
      /*   } */
      /* } */
    }
    /* putchar('\n'); */
  }

  int pos = 0;
  size_t checksum = 0;
  for (int i = 0; i < v.len; i++) {
    if (v.array[i].len == 0) {
      continue;
    }
    for (int j = 0; j < v.array[i].len; j++) {
      if (v.array[i].free_space) {
      } else {
        checksum += pos * v.array[i].id;
      }
      pos++;
    }
  }

  printf("%zu\n", checksum);

  vector_Pair_t_free(&v);
}
