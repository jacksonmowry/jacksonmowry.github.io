#include "rbuf.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define CAP 5

int main() {
    struct RingBuffer* rb = rb_new(CAP);
    char bytes[CAP];
    size_t n;
    char c;

    rb_push(rb, 'A');
    rb_push(rb, 'l');
    rb_push(rb, 'p');
    rb_push(rb, 'h');
    rb_push(rb, 'a');
    rb_push(rb, 'D');
    rb_push(rb, 'o');
    rb_push(rb, 'g');

    rb_pop(rb, &c);
    assert(rb_at(rb) == 1);
    assert(c == 'A');
    assert(rb_size(rb) == 4);

    n = rb_read(rb, bytes, CAP + 5);
    assert(n == 4);
    assert(!strncmp("lpha", bytes, 4));
    assert(rb_size(rb) == 0);

    n = rb_write(rb, "Hello World", strlen("Hello World"));
    assert(n == 5);
    assert(rb_size(rb) == 5);
    assert(rb_at(rb) == 0);
    assert(rb_peek(rb) == 'H');

    n = rb_read(rb, bytes, CAP + 10);
    assert(n == 5);
    assert(!strncmp("Hello", bytes, 5));
    assert(rb_size(rb) == 0);
    assert(rb_at(rb) == 0);

    rb_free(rb);

    return 0;
}
