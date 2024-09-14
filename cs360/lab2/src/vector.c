// Jackson Mowry
// Sep 8 2024
// A general purpose vector to learn about dynamic memory allocation and
// lifetimes in C
#include "vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A simple helper macro to check for a null vector before performing any
// operations.
#define VECTOR_NULL_CHECK(v)                                                   \
    if (!v) {                                                                  \
        fprintf(stderr, "Attempting to call '%s' on a NULL vector\n",          \
                __func__);                                                     \
        exit(1);                                                               \
    }

// Define the structure only in our .c file so that its implementation is hidden
// from users of our library.
typedef struct Vector {
    int size;
    int capacity;
    int64_t* values;
} Vector;

// vector_new allocates a vector of size 0, returning a pointer to the newly
// allocated structure.
Vector* vector_new(void) { return vector_new_with_capacity(0); }

// vector_new_with_capacity allocates a vector with the specified capacity,
// returning a pointer to the newly allocated structure.
Vector* vector_new_with_capacity(int capacity) {
    Vector* v = (Vector*)malloc(sizeof(Vector));
    // We have 2 possible points of failure within this function, both resulting
    // from calls to allocate memory.
    if (!v) {
        fprintf(stderr, "Failed to allocate memory for a vector!\n");
        exit(1);
    }

    v->size = 0;
    v->capacity = capacity;
    v->values = (int64_t*)calloc((size_t)v->capacity, sizeof(int64_t));

    // The second spot where memory allocating can fail.
    if (!v->values) {
        fprintf(stderr, "Failed to allocate memory for a vectors elements!\n");
        exit(1);
    }

    return v;
}

// vector_free first frees the backing array, then the heap allocated structure
// itself. Vector* v is no longer valid for use after returning.
void vector_free(Vector* v) {
    if (!v) {
        return;
    }

    free(v->values);
    v->values = NULL;
    free(v);
}

// vector_push adds a single 64-bit value to the end of the vector.
// value is passed by value.
// Each call to vector_push resizes the vector by 'size + 1'
void vector_push(Vector* v, int64_t value) {
    VECTOR_NULL_CHECK(v)

    vector_resize(v, v->size + 1);

    assert(v->size <= v->capacity);

    v->values[v->size - 1] = value;
}

// vector_insert places the provided value at the specified index, shifting all
// values to the right down by one. If index is past the end of the array it
// will be placed at the end.
void vector_insert(Vector* v, int index, int64_t value) {
    VECTOR_NULL_CHECK(v)

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
            ((size_t)(v->size - index) * sizeof(int64_t)));
    v->values[index] = value;
    v->size += 1;
}

// vector_removes removes the value at the provided index, shifting all values
// to the right back one place.
bool vector_remove(Vector* v, int index) {
    VECTOR_NULL_CHECK(v)

    if (index < 0) {
        fprintf(stderr, "Attempting to remove with index '%d'\n", index);
        exit(1);
    }

    if (index > v->size - 1) {
        return false;
    }

    assert(index >= 0 && index < v->size - 1);

    memmove(&v->values[index], &v->values[index + 1],
            ((size_t)(v->size - index + 1) * sizeof(int64_t)));
    v->size -= 1;

    return true;
}

// vector_get places the value at the specified index in the out parameter
// 'value'. If this operation is successful the function returns true, otherwise
// if the provided index is out of range false will be returned.
bool vector_get(Vector* v, int index, int64_t* value) {
    VECTOR_NULL_CHECK(v)

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

// vector_set updates the value as the specified index, returning true if that
// value was successfully updated.
bool vector_set(Vector* v, int index, int64_t value) {
    VECTOR_NULL_CHECK(v)

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

// vector_find performs a linear search returning the index of the first
// occurance of 'value'.
int vector_find(Vector* v, int64_t value) {
    VECTOR_NULL_CHECK(v)

    for (int i = 0; i < v->size; i++) {
        if (value == v->values[i]) {
            return true;
        }
    }

    return false;
}

// I am assuming we are intended to include this funciton?
// comp_ascending takes in 2 int64_t values by value, and returns if they are in
// ascending order.
static bool comp_ascending(int64_t left, int64_t right) {
    return left <= right;
}

// vector_sort sorts a vector in ascending order by int64_t values.
void vector_sort(Vector* v) {
    VECTOR_NULL_CHECK(v)

    vector_sort_by(v, comp_ascending);
}

// vector_sort_by sorts a vector by the provided sorting function.
// 'comp' should have the function signature bool (*comp)(int64_t, int64_t),
// returning true if the provided values are already "in order".
void vector_sort_by(Vector* v, bool (*comp)(int64_t left, int64_t right)) {
    VECTOR_NULL_CHECK(v)

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

// vector_bsearch takes in a sorted vector, returning the index of the provided
// value, or -1 if it is not found.
int vector_bsearch(Vector* v, int64_t value) {
    VECTOR_NULL_CHECK(v)

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

// vector_resize adjusts the size field of a vector, reallocating the backing
// storage/capacity if needed.
void vector_resize(Vector* v, int new_size) {
    VECTOR_NULL_CHECK(v)

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

// vector_reserve adjusts the capacity/backing storage of a vector, reallocating
// if necessary.
void vector_reserve(Vector* v, int new_capacity) {
    VECTOR_NULL_CHECK(v)

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
    v->values = malloc((size_t)v->capacity * sizeof(int64_t));
    if (!v->values) {
        fprintf(stderr, "Attempt to reallocate vector values failed!\n");
        exit(1);
    }

    // Sometimes malloc'ing a larger chunk returns the same underlying spot
    // in memory so we can skip copying over memory to the same location
    // Also it would be unsafe to use memcpy in case of the overlapping
    // memory regions, but Marz says we should use memcpy for speed
    if (tmp != v->values) {
        memcpy(v->values, tmp, (size_t)(v->size * sizeof(int64_t)));
    }

    free(tmp);
}

// vector_clear sets a vectors size to 0.
void vector_clear(Vector* v) {
    VECTOR_NULL_CHECK(v)

    v->size = 0;
}

// vector_capacity returns the capacity field of a vector
int vector_capacity(Vector* vec) { return vec->capacity; }

// vector_size returns the size field of a vector
int vector_size(Vector* vec) { return vec->size; }
