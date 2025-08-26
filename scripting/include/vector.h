#pragma once

#include "allocator.h"
#include <assert.h>
#include <stddef.h>

#include "fat_pointer.h"

/*********************************/
/* generic vector implementation */
/*********************************/
#define Vector(type)                                                           \
    struct {                                                                   \
        FatPointer(type) fat_ptr;                                              \
        size_t size;                                                           \
        Allocator allocator;                                                   \
    }

#define vector_pb(v, item)                                                     \
    assert(v.fat_ptr.ptr);                                                     \
    if (v.size >= v.fat_ptr.size) {                                            \
        auto fat_p = reallocate(v.allocator, v.fat_ptr, v.fat_ptr.size * 2);   \
        memcpy((void*)&v.fat_ptr, (void*)&fat_p, sizeof(fat_p));               \
    }                                                                          \
    v.fat_ptr.ptr[v.size++] = item;

#define vector_init(T, allocator, capacity)                                    \
    (Vector(T)) {                                                              \
        .fat_ptr = allocate_many(allocator, T, capacity), .size = 0,           \
        .allocator = allocator,                                                \
    }
