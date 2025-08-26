#pragma once

#define FatPointer(T)                                                          \
    struct {                                                                   \
        T* ptr;                                                                \
        size_t len;                                                            \
    }

#define fat_ptr_memcpy(destination, source)                                    \
    memcpy(destination.ptr, source.ptr, sizeof(*source.ptr) * source.len)
#define fat_ptr_memcmp(destination, source)                                    \
    ((destination.len == source.len) &&                                        \
     (!memcmp(destination.ptr, source.ptr, sizeof(*source.ptr) * source.len)))
