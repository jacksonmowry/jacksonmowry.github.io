#include "allocator.h"

void* reallocate_impl(Allocator a, void* prev_ptr, size_t element_size,
                      size_t prev_element_count, size_t new_element_count) {
    if (new_element_count < prev_element_count) {
        return prev_ptr;
    }

    void* new_ptr =
        a.vtable.allocate_many_virtual(a, element_size, new_element_count);

    if (!new_ptr) {
        return nullptr;
    }

    memcpy(new_ptr, prev_ptr, prev_element_count * element_size);
    a.vtable.deallocate_virtual(a, prev_ptr, element_size, prev_element_count);

    return new_ptr;
}
