#pragma once

#include <stddef.h>
#include <string.h>

#include "fat_pointer.h"

typedef struct Allocator {
    void* data;
    struct {
        void* (*allocate_many_virtual)(struct Allocator this, size_t size,
                                       size_t count);
        void (*deallocate_virtual)(struct Allocator this, void* old_ptr,
                                   size_t element_size, size_t element_count);
        void (*deinit)(struct Allocator this);
    } vtable;
} Allocator;

void* reallocate_impl(Allocator a, void* prev_ptr, size_t element_size,
                      size_t prev_element_count, size_t new_element_count);

#define allocate(allocator, T, current)                                        \
    (T*)allocator.vtable.allocate_many_virtual(allocator, sizeof(T), 1)
#define allocate_many(allocator, T, count)                                     \
    (FatPointer(T)) {                                                          \
        .ptr = (T*)allocator.vtable.allocate_many_virtual(allocator,           \
                                                          sizeof(T), count),   \
        .len = count                                                           \
    }
#define reallocate(allocator, prev_fat_ptr, new_element_count)                 \
    (FatPointer(typeof(*prev_fat_ptr.ptr))) {                                  \
        .ptr = reallocate_impl(allocator, prev_fat_ptr.ptr,                    \
                               sizeof(*prev_fat_ptr.ptr), prev_fat_ptr.len,    \
                               new_element_count),                             \
        .len = new_element_count                                               \
    }
#define deallocate(allocator, old_fat_ptr)                                     \
    allocator.vtable.deallocate_virtual(                                       \
        allocator, old_fat_ptr.ptr, sizeof(*old_fat_ptr.ptr), old_fat_ptr.len)
