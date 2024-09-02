#include "vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_NULL_CHECK(v)                                                   \
    if (!v) {                                                                  \
        fprintf(stderr, "Attempting to call '%s' on a NULL vector\n",          \
                __func__);                                                     \
        exit(1);                                                               \
    }

typedef struct Vector {
    int size;
    int capacity;
    int64_t* values;
} Vector;

#if DEBUG
void vector_dump(Vector* v) {
    printf("[");
    for (int i = 0; i < v->size - 1; i++) {
        printf("%ld ", v->values[i]);
    }
    if (v->size > 1) {
        printf("%ld]\n", v->values[v->size - 1]);
    } else {
        printf("]\n");
    }
}
#else
void vector_dump(Vector* v) {}
#endif

Vector* vector_new(void) {
    Vector* v = (Vector*)calloc(1, sizeof(Vector));
    if (!v) {
        fprintf(stderr, "Attempting to allocate a new vector failed!\n");
        exit(1);
    }

    *v = (Vector){.size = 0, .capacity = 0, .values = NULL};

    return v;
}

Vector* vector_new_with_capacity(int capacity) {
    Vector* v = vector_new();
    v->capacity = capacity;
    v->values = (int64_t*)calloc(capacity, sizeof(int64_t));

    return v;
}

void vector_free(Vector* v) {
    if (!v) {
        return;
    }

    free(v->values);
    v->values = NULL;
    free(v);
}

void vector_push(Vector* v, int64_t value) {
    VECTOR_NULL_CHECK(v);

    vector_resize(v, v->size + 1);

    assert(v->size <= v->capacity);

    v->values[v->size - 1] = value;
}

void vector_insert(Vector* v, int index, int64_t value) {
    VECTOR_NULL_CHECK(v);

    if (index < 0) {
        fprintf(stderr, "Attempting to insert with index '%d'\n", index);
        exit(1);
    }

    if (index >= v->size) {
        vector_push(v, value);
        return;
    }

    assert(index >= 0 && index < v->size);

    if (v->size + 1 > v->capacity) {
        vector_reserve(v, v->capacity * 2);
    }

    memmove(&v->values[index + 1], &v->values[index],
            (v->size - index) * sizeof(int64_t));
    v->values[index] = value;
    v->size += 1;
}

bool vector_remove(Vector* v, int index) {
    VECTOR_NULL_CHECK(v);

    if (index < 0) {
        fprintf(stderr, "Attempting to remove with index '%d'\n", index);
        exit(1);
    }

    if (index > v->size - 1) {
        return false;
    }

    assert(index >= 0 && index < v->size - 1);

    memmove(&v->values[index], &v->values[index + 1],
            (v->size - index + 1) * sizeof(int64_t));
    v->size -= 1;

    return true;
}

bool vector_get(Vector* v, int index, int64_t* value) {
    VECTOR_NULL_CHECK(v);

    if (index < 0) {
        fprintf(stderr, "Attempting to get with index '%d'\n", index);
        exit(1);
    }

    if (index > v->size - 1) {
        value = NULL;
        return false;
    }

    assert(index >= 0 && index <= v->size - 1);

    *value = v->values[index];
    return true;
}

bool vector_set(Vector* v, int index, int64_t value) {
    VECTOR_NULL_CHECK(v);

    if (index < 0) {
        fprintf(stderr, "Attempting to set with index '%d'\n", index);
        exit(1);
    }

    if (index > v->size - 1) {
        return false;
    }

    assert(index >= 0 && index < v->size - 1);

    v->values[index] = value;
    return true;
}

int vector_find(Vector* v, int64_t value) {
    VECTOR_NULL_CHECK(v);

    for (int i = 0; i < v->size; i++) {
        if (value == v->values[i]) {
            return true;
        }
    }

    return false;
}

static bool comp_ascending(int64_t left, int64_t right) {
    return left <= right;
}

void vector_sort(Vector* v) {
    VECTOR_NULL_CHECK(v);

    vector_sort_by(v, comp_ascending);
}

void vector_sort_by(Vector* v, bool (*comp)(int64_t left, int64_t right)) {
    VECTOR_NULL_CHECK(v);

    int i;
    int j;
    int min;
    int64_t tmp;

    for (i = 0; i < v->size - 1; i++) {
        min = i;
        for (j = i + 1; j < v->size; j++) {
            if (!comp(v->values[min], v->values[j])) {
                min = j;
            }
        }
        if (min != i) {
            tmp = v->values[min];
            v->values[min] = v->values[i];
            v->values[i] = tmp;
        }
    }
}

int vector_bsearch(Vector* v, int64_t value) {
    VECTOR_NULL_CHECK(v);

    int left = 0;
    int right = v->size - 1;

    while (left <= right) {
        int mid = ((right - left) / 2) + left;

        if (v->values[mid] == value) {
            return mid;
        } else if (v->values[mid] < value) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return -1;
}

void vector_resize(Vector* v, int new_size) {
    VECTOR_NULL_CHECK(v);

    if (new_size < 0) {
        fprintf(stderr, "Attempting to resize vector with size '%d'\n",
                new_size);
        exit(1);
    }

    if (new_size <= v->capacity) {
        v->size = new_size;
        return;
    }

    vector_reserve(v, new_size > v->capacity * 2 ? new_size : v->capacity * 2);
    v->size = new_size;
}

void vector_reserve(Vector* v, int new_capacity) {
    VECTOR_NULL_CHECK(v);

    if (new_capacity < 0) {
        fprintf(stderr, "Attempting to reserve vector with capacity '%d'\n",
                new_capacity);
        exit(1);
    }

    if (new_capacity < v->size) {
        v->capacity = v->size;
        return;
    }

    int64_t* tmp = v->values;

    v->capacity = new_capacity;
    v->values = (int64_t*)calloc(v->capacity, sizeof(int64_t));
    if (!v->values) {
        fprintf(stderr, "Attempt to reallocate vector values failed!\n");
        exit(1);
    }

    memmove(v->values, tmp, v->size * sizeof(int64_t));

    free(tmp);
}

void vector_clear(Vector* v) {
    VECTOR_NULL_CHECK(v);

    v->size = 0;
}

int vector_capacity(Vector* vec) { return vec->capacity; }

int vector_size(Vector* vec) { return vec->size; }
