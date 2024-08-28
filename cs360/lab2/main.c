#include "vector.h"
#include <assert.h>
#include <stdio.h>

static bool comp_descending(int64_t left, int64_t right) {
    return left > right;
}

int main() {
    Vector* v = vector_new_with_capacity(10);
    assert(v);
    assert(vector_size(v) == 0);
    assert(vector_capacity(v) == 10);

    for (int64_t i = 0; i < 15; i++) {
        vector_push(v, i);
    vector_dump(v);
    }

    vector_dump(v);

    assert(vector_size(v) == 15);
    assert(vector_capacity(v) == 20);

    int64_t value;
    assert(vector_get(v, 14, &value));
    printf("%ld\n", value);
    assert(value == 14);

    int64_t val;
    vector_dump(v);

    vector_insert(v, 0, 42);

    vector_dump(v);

    vector_insert(v, 6, 69);

    vector_dump(v);

    vector_insert(v, 7, 11);
    vector_insert(v, 2, 11);
    vector_insert(v, 8, 11);
    vector_insert(v, 2, 11);
    vector_insert(v, 8, 11);
    vector_insert(v, 7, 11);

    vector_dump(v);

    vector_sort(v);

    vector_dump(v);

    vector_get(v, vector_size(v)-2, &val);
    assert(val == 42);

    vector_remove(v, vector_size(v)-2);
    vector_dump(v);

    vector_get(v, vector_size(v)-2, &val);
    assert(val == 14);

    vector_dump(v);

    vector_resize(v, 5);
    assert(vector_size(v) == 5);
    vector_dump(v);

    vector_reserve(v, 3);
    assert(vector_size(v) == 5);
    assert(vector_capacity(v) == 5);
    vector_dump(v);

    vector_resize(v, 20);
    assert(vector_size(v) == 20);
    assert(vector_capacity(v) == 20);
    vector_dump(v);

    vector_set(v, 0, 7);
    vector_get(v, 0, &val);
    assert(val == 7);

    vector_dump(v);

    vector_sort_by(v, comp_descending);
    int64_t another_val;
    vector_get(v, 0, &val);
    vector_get(v, vector_size(v)-1, &another_val);

    assert(val >= another_val);

    vector_dump(v);

    vector_push(v, 69);
    int index = vector_bsearch(v, 69);
    assert(index == vector_size(v)-1);
    vector_dump(v);

    vector_clear(v);
    assert(vector_size(v) == 0);
    vector_dump(v);

    vector_free(v);
}
