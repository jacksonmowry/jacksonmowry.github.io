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

typedef struct Pair {
  int row;
  int col;
} Pair_t;

Pair_t pair_dist(const Pair_t a, const Pair_t b) {
  return (Pair_t){.row = (a.row - b.row), .col = (a.col - b.col)};
}

Pair_t pair_mul(const Pair_t a, int mul) {
  return (Pair_t){.row = a.row * mul, .col = a.col * mul};
}

bool pair_eq(const Pair_t a, const Pair_t b) {
  return a.row == b.row && a.col == b.col;
}

VECTOR_PROTOTYPE(Pair_t);
VECTOR(Pair_t);

vector_Pair_t map[128];

int main() {
  vector_Pair_t good = vector_Pair_t_init(10);
  for (int i = 0; i < 128; i++) {
    map[i] = vector_Pair_t_init(10);
  }

  int row = 0;
  char line[256];

  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = '\0';
    for (int col = 0; line[col]; col++) {
      if (line[col] == '.') {
        continue;
      }

      vector_Pair_t_pb(&map[line[col]], (Pair_t){.row = row, .col = col});
    }

    row++;
  }

  /* for (int i = 0; i < 128; i++) { */
  /*   if (map[i].len != 0) { */
  /*     printf("%c: ", i); */
  /*   } */
  /*   for (int j = 0; j < map[i].len; j++) { */
  /*     printf("(%d,%d) ", map[i].array[j].row, map[i].array[j].col); */
  /*   } */
  /*   if (map[i].len != 0) { */
  /*     putchar('\n'); */
  /*   } */
  /* } */
  /* putchar('\n'); */

  int counter = 0;

  for (int letter = 0; letter < 128; letter++) {
    if (map[letter].len > 1) {
      for (int i = 0; i < map[letter].len; i++) {
        vector_Pair_t_pb(&good, map[letter].array[i]);
      }
    }
    for (int i = 0; i < map[letter].len; i++) {
      const Pair_t a = map[letter].array[i];

      for (int j = i + 1; j < map[letter].len; j++) {
        const Pair_t b = map[letter].array[j];

        // find possible positions for double one, single other
        for (int y = 0; y < row; y++) {
          for (int x = 0; x < strlen(line); x++) {
            const Pair_t cur_pos = {.row = y, .col = x};

            if (pair_eq(cur_pos, a) || pair_eq(cur_pos, b)) {
              continue;
            }

            const Pair_t pos_a_dist = pair_dist(cur_pos, a);
            const Pair_t pos_b_dist = pair_dist(cur_pos, b);

            float row_mul = (float)pos_a_dist.row / (float)pos_b_dist.row;
            float col_mul = (float)pos_a_dist.col / (float)pos_b_dist.col;

            if (row_mul == col_mul) {
              /* printf("%c: (%d,%d), %f %f\n", letter, y, x, row_mul, col_mul);
               */
              counter++;
              vector_Pair_t_pb(&good, cur_pos);
            }
          }
        }
      }
    }
  }

  for (int i = 0; i < good.len; i++) {
    printf("(%d,%d)\n", good.array[i].row, good.array[i].col);
  }

  vector_Pair_t_free(&good);

  /* printf("%d\n", counter); */
}
