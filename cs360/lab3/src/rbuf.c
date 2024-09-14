#include "rbuf.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define RB_NULL_CHECK(rb)                                                      \
    if (!rb) {                                                                 \
        fprintf(stderr, "Attempting to call '%s' on a NULL ringbuffer\n",      \
                __func__);                                                     \
        exit(1);                                                               \
    }

typedef struct RingBuffer {
    size_t at;       // Index of the next byte to be read
    size_t size;     // The total number of unread bytes in the ring buffer
    size_t capacity; // The total number of bytes that can be stored in this
                     // ring buffer
    char* buffer; // The actual data storage. Must be created with calloc using
                  // capacity as the number of elements to create
} RingBuffer;

struct RingBuffer* rb_new(size_t capacity) {
    struct RingBuffer* rb;
    if (!(rb = (struct RingBuffer*)malloc(sizeof(*rb)))) {
        // Out of memory.
        return NULL;
    }
    // If we get here, all is well with the memory.
    rb->at = 0;
    rb->size = 0;
    rb->capacity = capacity;
    rb->buffer = (char*)malloc(rb->capacity * sizeof(char));
    if (!rb->buffer) {
        // We were able to create the structure in memory,
        // but not the buffer. We need both, so free the
        // structure and return NULL (error).
        free(rb);
        return NULL;
    }
    return rb;
}

void rb_free(struct RingBuffer* rb) {
    free(rb->buffer);
    free(rb);
}

size_t rb_at(const struct RingBuffer* rb) { return rb->at; }

size_t rb_size(const struct RingBuffer* rb) { return rb->size; }

size_t rb_capacity(const struct RingBuffer* rb) { return rb->capacity; }

bool rb_push(struct RingBuffer* rb, char data) {
    RB_NULL_CHECK(rb);

    if (rb->size >= rb->capacity) {
        return false;
    }

    rb->buffer[(rb->at + (rb->size++)) % rb->capacity] = data;
    return true;
}

bool rb_pop(struct RingBuffer* rb, char* data) {
    RB_NULL_CHECK(rb);

    if (rb->size == 0) {
        return false;
    }

    char popped_data = rb->buffer[rb->at++];
    rb->size--;

    rb->at %= rb->capacity;

    if (data) {
        *data = popped_data;
    }

    return true;
}

char rb_peek(const struct RingBuffer* rb) {
    RB_NULL_CHECK(rb);

    if (rb->size == 0) {
        return 0;
    }

    return rb->buffer[rb->at % rb->capacity];
}

void rb_ignore(struct RingBuffer* rb, size_t num) {
    RB_NULL_CHECK(rb);

    if (num > rb->size) {
        rb->at = (rb->at - rb->size) % rb->capacity;
        rb->size = 0;
    } else {
        rb->size -= num;
        rb->at = (rb->at - num) % rb->capacity;
    }
}

size_t rb_read(struct RingBuffer* rb, char* buf, size_t max_bytes) {
    max_bytes = max_bytes > rb->size ? rb->size : max_bytes;
    size_t bytes_to_end = rb->capacity - rb->at;
    size_t bytes_from_start = rb->at + 1;

    size_t first_copy = bytes_to_end > max_bytes ? max_bytes : bytes_to_end;
    max_bytes -= first_copy;
    size_t second_copy =
        bytes_from_start > max_bytes ? max_bytes : bytes_from_start;

    assert(first_copy + second_copy <= max_bytes + first_copy);

    memcpy(buf, &rb->buffer[rb->at], first_copy);
    memcpy(buf + first_copy, &rb->buffer[0], second_copy);

    rb->size -= first_copy + second_copy;
    rb->at = (rb->at + first_copy + second_copy) % rb->capacity;

    return first_copy + second_copy;
}

size_t rb_write(struct RingBuffer* rb, const char* buf, size_t bytes) {
    size_t space_avail =
        bytes > (rb->capacity - rb->size) ? (rb->capacity - rb->size) : bytes;

    size_t bytes_to_end = rb->capacity - rb->at;
    size_t bytes_from_start = rb->at + 1;

    size_t first_copy = bytes_to_end > space_avail ? space_avail : bytes_to_end;
    space_avail -= first_copy;
    size_t second_copy =
        bytes_from_start > space_avail ? space_avail : bytes_from_start;

    assert(first_copy + second_copy <= space_avail + first_copy);

    memcpy(rb->buffer, buf, first_copy);
    memcpy(&rb->buffer[(rb->at + first_copy) % rb->capacity], buf + first_copy,
           second_copy);

    rb->size += first_copy + second_copy;

    return first_copy + second_copy;
}

void rb_clear(struct RingBuffer* rb) {
    rb->at = 0;
    rb->size = 0;
}
