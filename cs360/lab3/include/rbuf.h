#pragma once
#include <stdbool.h>
#include <stdlib.h>
// Opaque structure defined in the C file
struct RingBuffer;
typedef struct RingBuffer RingBuffer;

// Memory management
RingBuffer *rb_new(size_t capacity);
void rb_free(RingBuffer *rb);

// Ring buffer values
size_t rb_at(const RingBuffer *rb);
size_t rb_size(const RingBuffer *rb);
size_t rb_capacity(const RingBuffer *rb);

// Individual item functions
bool rb_push(RingBuffer *rb, char data);
bool rb_pop(RingBuffer *rb, char *data);
char rb_peek(const RingBuffer *rb);
void rb_ignore(RingBuffer *rb, size_t num);

// Buffer functions
size_t rb_read(RingBuffer *rb, char *buf, size_t max_bytes);
size_t rb_write(RingBuffer *rb, const char *buf, size_t bytes);
void rb_clear(RingBuffer *rb);
