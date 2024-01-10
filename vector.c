#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define vector(type) vector_##type

#define vector_init(v, amount, val)                                            \
  _Generic((v)->data, int *: vector_int_init, float *: vector_float_init)(     \
      v, amount, val)

// `v` is unsafe to use after calling unless reinitialized
#define vector_destroy(v)                                                      \
  _Generic((v)->data,                                                          \
      int *: vector_int_destroy,                                               \
      float *: vector_float_destroy)(v)

#define vector_push(v, val)                                                    \
  _Generic((v)->data, int *: vector_int_pb, float *: vector_float_pb)(v, val)

#define vector_pop(v)                                                          \
  _Generic((v)->data, int *: vector_int_pop, float *: vector_float_pop)(v)

#define vector_insert(v, pos, val)                                             \
  _Generic((v)->data, int *: vector_int_ins, float *: vector_float_ins)(       \
      v, pos, val)

#define vector_delete(v, pos)                                                  \
  _Generic((v)->data, int *: vector_int_del, float *: vector_float_del)(v, pos)

#define vector_str(v)                                                          \
  _Generic((v)->data, int *: vector_int_str, float *: vector_float_str)(v)

#define vector_print(v)                                                        \
  _Generic((v)->data, int *: vector_int_print, float *: vector_float_print)(v)

#define vector_clear(v)                                                        \
  _Generic((v)->data, int *: vector_int_clear, float *: vector_float_clear)(v)

#define vector_stf(v)                                                          \
  _Generic((v)->data, int *: vector_int_stf, float *: vector_float_stf)(v)

#define vector_sort(v)                                                         \
  _Generic((v)->data, int *: vector_int_sort, float *: vector_float_sort)(v)

#define vector_reverse(v)                                                      \
  _Generic((v)->data, int *: vector_int_rev, float *: vector_float_rev)(v)

#define vector_concat(v, w)                                                    \
  _Generic((v)->data,                                                          \
      int *: _Generic((w)->data, int *: vector_int_concat, default: error),    \
      float *: _Generic((w)->data,                                             \
      float *: vector_float_concat,                                            \
      default: error))(v, w)

void error() { assert(0); }

typedef struct vector_int {
  int *data;
  size_t size;
  size_t capacity;
} vector_int;

typedef struct vector_float {
  float *data;
  size_t size;
  size_t capacity;
} vector_float;

bool vector_int_init(vector_int *v, size_t amount, typeof(*(v->data)) val) {
  v->data = malloc(amount * sizeof(typeof(*(v->data))));
  if (!v->data) {
    perror("malloc");
    return false;
  }
  for (size_t i = 0; i < amount; i++) {
    v->data[i] = val;
  }
  v->size = amount;
  v->capacity = amount;
  return true;
}

bool vector_float_init(vector_float *v, size_t amount, typeof(*(v->data)) val) {
  v->data = malloc(amount * sizeof(typeof(*(v->data))));
  if (!v->data) {
    perror("malloc");
    return false;
  }
  for (size_t i = 0; i < amount; i++) {
    v->data[i] = val;
  }
  v->size = amount;
  v->capacity = amount;
  return true;
}

// `v` is unsafe to use after calling unless reinitialized
void vector_int_destroy(vector_int *v) {
  if (!v) { return; }
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

// `v` is unsafe to use after calling unless reinitialized
void vector_float_destroy(vector_float *v) {
  if (!v) { return; }
  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

bool vector_int_pb(vector_int *v, int val) {
  if (!v || !v->data) {
    return false;
  }
  if (v->size == v->capacity) {
    v->capacity *= 2;
    v->data = realloc(v->data, v->capacity * sizeof(typeof(*(v->data))));
    if (!v->data) {
      perror("realloc");
      return false;
    }
  }
  v->data[v->size++] = val;
  return true;
}

bool vector_float_pb(vector_float *v, float val) {
  if (!v || !v->data) {
    return false;
  }
  if (v->size == v->capacity) {
    v->capacity *= 2;
    v->data = realloc(v->data, v->capacity * sizeof(typeof(*(v->data))));
    if (!v->data) {
      perror("realloc");
      return false;
    }
  }
  v->data[v->size++] = val;
  return true;
}

int vector_int_pop(vector_int *v) {
  if (!v || !v->data) {
    return 0;
  }
  if (v->size == 0) {
    return 0;
  }
  return v->data[--v->size];
}

float vector_float_pop(vector_float *v) {
  if (!v || !v->data) {
    return 0;
  }
  if (v->size == 0) {
    return 0;
  }
  return v->data[--v->size];
}

bool vector_int_ins(vector_int *v, size_t pos, typeof(*(v->data)) val) {
  if (!v || !v->data) {
    return false;
  }
  if (pos >= v->size) {
    return vector_push(v, val);
  }
  if (v->size + 1 >= v->capacity) {
    v->capacity *= 2;
    v->data = realloc(v->data, v->capacity * sizeof(typeof(*(v->data))));
    if (!v->data) {
      perror("realloc");
      return false;
    }
  }
  memmove(&v->data[pos + 1], &v->data[pos],
          (v->size - pos) * sizeof(typeof(*(v->data))));
  v->data[pos] = val;
  v->size += 1;
  return true;
}

bool vector_float_ins(vector_float *v, size_t pos, typeof(*(v->data)) val) {
  if (!v || !v->data) {
    return false;
  }
  if (pos >= v->size) {
    return vector_push(v, val);
  }
  if (v->size + 1 >= v->capacity) {
    v->capacity *= 2;
    v->data = realloc(v->data, v->capacity * sizeof(typeof(*(v->data))));
    if (!v->data) {
      perror("realloc");
      return false;
    }
  }
  memmove(&v->data[pos + 1], &v->data[pos],
          (v->size - pos) * sizeof(typeof(*(v->data))));
  v->data[pos] = val;
  v->size += 1;
  return true;
}

int vector_int_del(vector_int *v, size_t pos) {
  if (!v || !v->data) {
    return 0;
  }
  if (pos >= v->size - 1) {
    return vector_pop(v);
  }
  typeof(*(v->data)) val = v->data[pos];
  memmove(&v->data[pos], &v->data[pos + 1],
          (v->size - pos - 1) * sizeof(typeof(*(v->data))));
  v->size -= 1;
  return val;
}

float vector_float_del(vector_float *v, size_t pos) {
  if (!v || !v->data) {
    return 0;
  }
  if (pos >= v->size - 1) {
    return vector_pop(v);
  }
  typeof(*(v->data)) val = v->data[pos];
  memmove(&v->data[pos], &v->data[pos + 1],
          (v->size - pos - 1) * sizeof(typeof(*(v->data))));
  v->size -= 1;
  return val;
}

__attribute__((warn_unused_result)) char *vector_int_str(vector_int *v) {
  if (!v || !v->data) {
    return NULL;
  }
  char *buf = calloc(100, 1);
  size_t cap = 100;
  size_t size = 0;
  buf[size++] = '[';
  for (size_t i = 0; i < v->size; i++) {
    char temp[14];
    int temp_size = sprintf(temp, "%d", v->data[i]);
    if (size + temp_size >= cap) {
      cap *= 2;
      buf = realloc(buf, cap);
      if (!buf) {
        perror("realloc");
        return NULL;
      }
    }
    strncpy(&buf[size], temp, temp_size);
    size += temp_size;
    if (i != v->size - 1) {
      strcpy(&buf[size], ", ");
      size += 2;
    }
  }
  buf[size++] = ']';
  buf[size] = '\0';
  return buf;
}

__attribute__((warn_unused_result)) char *vector_float_str(vector_float *v) {
  if (!v || !v->data) {
    return NULL;
  }
  char *buf = calloc(100, 1);
  size_t cap = 100;
  size_t size = 0;
  buf[size++] = '[';
  for (size_t i = 0; i < v->size; i++) {
    char temp[14];
    int temp_size = sprintf(temp, "%.2f", v->data[i]);
    if (size + temp_size >= cap) {
      cap *= 2;
      buf = realloc(buf, cap);
      if (!buf) {
        perror("realloc");
        return NULL;
      }
    }
    strncpy(&buf[size], temp, temp_size);
    size += temp_size;
    if (i != v->size - 1) {
      strcpy(&buf[size], ", ");
      size += 2;
    }
  }
  buf[size++] = ']';
  buf[size] = '\0';
  return buf;
}

void vector_int_print(vector_int *v) {
  if (!v || !v->data) {
    return;
  }
  char *str = vector_str(v);
  puts(str);
  free(str);
}

void vector_float_print(vector_float *v) {
  if (!v || !v->data) {
    return;
  }
  char *str = vector_str(v);
  puts(str);
  free(str);
}

bool vector_int_clear(vector_int *v) {
  if (!v || !v->data) {
    return false;
  }
  v->size = 0;
  return true;
}

bool vector_float_clear(vector_float *v) {
  if (!v || !v->data) {
    return false;
  }
  v->size = 0;
  return true;
}

bool vector_int_stf(vector_int *v) {
  if (!v || !v->data) {
    return false;
  }
  v->capacity = v->size;
  v->data = realloc(v->data, v->size * sizeof(typeof(*(v->data))));
  if (!v->data) {
    perror("realloc");
    return false;
  }
  return true;
}

bool vector_float_stf(vector_float *v) {
  if (!v || !v->data) {
    return false;
  }
  v->capacity = v->size;
  v->data = realloc(v->data, v->size * sizeof(typeof(*(v->data))));
  if (!v->data) {
    perror("realloc");
    return false;
  }
  return true;
}

int vector_int_comp(const void *f, const void *s) {
  int a = *(int *)f;
  int b = *(int *)s;
  if (a < b) {
    return -1;
  } else if (a > b) {
    return 1;
  } else {
    return 0;
  }
}

int vector_float_comp(const void *f, const void *s) {
  int a = *(float *)f;
  int b = *(float *)s;
  if (a < b) {
    return -1;
  } else if (a > b) {
    return 1;
  } else {
    return 0;
  }
}

void vector_int_sort(vector_int *v) {
  if (!v || !v->data) {
    return;
  }
  qsort(v->data, v->size, sizeof(typeof(*(v->data))), vector_int_comp);
}

void vector_float_sort(vector_float *v) {
  if (!v || !v->data) {
    return;
  }
  qsort(v->data, v->size, sizeof(typeof(*(v->data))), vector_float_comp);
}

bool vector_int_rev(vector_int *v) {
  if (!v || !v->data) {
    return NULL;
  }
  for (size_t i = 0; i < v->size / 2; i++) {
    typeof(*(v->data)) tmp = v->data[i];
    v->data[i] = v->data[v->size - 1 - i];
    v->data[v->size - 1 - i] = tmp;
  }
  return true;
}

bool vector_float_rev(vector_float *v) {
  if (!v || !v->data) {
    return NULL;
  }
  for (size_t i = 0; i < v->size / 2; i++) {
    typeof(*(v->data)) tmp = v->data[i];
    v->data[i] = v->data[v->size - 1 - i];
    v->data[v->size - 1 - i] = tmp;
  }
  return true;
}

bool vector_int_concat(vector_int *v, vector_int *w) {
  if (!v || !v->data || !w || !w->data) {
    return NULL;
  }
  if (v->size + w->size > v->capacity) {
    if (v->size + w->size > v->capacity * 2) {
      v->capacity = v->size + w->size;
    } else {
      v->capacity *= 2;
    }
    v->data = realloc(v->data, v->capacity * sizeof(typeof(*(v->data))));
    if (!v->data) {
      perror("realloc");
      return false;
    }
  }
  memcpy(&v->data[v->size], w->data, w->size * sizeof(typeof(*(v->data))));
  v->size += w->size;
  return true;
}

bool vector_float_concat(vector_float *v, vector_float *w) {
  if (!v || !v->data || !w || !w->data) {
    return NULL;
  }
  if (v->size + w->size > v->capacity) {
    if (v->size + w->size > v->capacity * 2) {
      v->capacity = v->size + w->size;
    } else {
      v->capacity *= 2;
    }
    v->data = realloc(v->data, v->capacity * sizeof(typeof(*(v->data))));
    if (!v->data) {
      perror("realloc");
      return false;
    }
  }
  memcpy(&v->data[v->size], w->data, w->size * sizeof(typeof(*(v->data))));
  v->size += w->size;
  return true;
}

int main() {
  vector(int) v;
  vector_init(&v, 10, 4);
  vector(int) w;
  vector_init(&w, 10, 5);

  vector_print(&v);
  vector_print(&w);
  puts("");

  vector(float) f;

  vector_push(&v, 5.0f);
  vector_print(&v);

  vector_concat(&v, &w);

  vector_print(&v);
  printf("size: %d, cap: %d\n", v.size, v.capacity);
  puts("");

  vector_destroy(&v);
  printf("%p\n", v.data);
}
