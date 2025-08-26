#include "arena.h"
#include <stdlib.h>

typedef struct Arena {
    void* base;
    void* current;
} Arena;

static void* alloc_many(Allocator this, size_t size, size_t count) {
    Arena* a = (Arena*)this.data;

    void* ret = a->current;

    a->current = (char*)a->current + (size * count);

    return ret;
}

static void dealloc(Allocator this, void* old_ptr, size_t element_size,
                    size_t element_count) {
    (void)this;
    (void)old_ptr;
    (void)element_size;
    (void)element_count;
}

static void deinit(Allocator this) {
    Arena* a = (Arena*)this.data;

    free(a);
}

Allocator arena_init(size_t bytes) {
    void* buf = malloc(bytes);
    Arena* a = buf;
    buf = ((Arena*)buf) + 1;

    a->base = buf;
    a->current = buf;

    return (Allocator){.data = a,
                       .vtable = {.allocate_many_virtual = alloc_many,
                                  .deallocate_virtual = dealloc,
                                  .deinit = deinit}};
}
