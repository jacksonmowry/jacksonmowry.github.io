#include "rbuf.h"
#include <assert.h>
#include <stdio.h>
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

    n = rb_read(rb, bytes, 999);
    assert(rb_at(rb) == 0);
    assert(rb_size(rb) == 0);
    assert(n == 0);

    rb_clear(rb);
    assert(rb_at(rb) == 0);
    assert(rb_size(rb) == 0);

    rb_push(rb, 'j');
    rb_push(rb, 'a');
    rb_push(rb, 'c');
    rb_push(rb, 'k');

    assert(rb_pop(rb, &c));
    assert(c == 'j');
    assert(rb_size(rb) == 3);
    assert(rb_at(rb) == 1);

    rb_push(rb, 's'); // should now be [ , a, c, k, s]
    assert(rb_size(rb) == 4);

    n = rb_read(rb, bytes, 4);
    assert(!strncmp("acks", bytes, 4));
    assert(n == 4);
    assert(rb_size(rb) == 0);
    assert(rb_at(rb) == 0);

    rb_clear(rb);

    assert(rb_push(rb, 'j'));
    assert(rb_push(rb, 'a'));
    assert(rb_push(rb, 'c'));
    assert(rb_push(rb, 'k'));
    assert(rb_push(rb, 's'));
    assert(rb_peek(rb) == 'j');
    assert(rb_size(rb) == 5);
    rb_ignore(rb, 4);
    assert(rb_size(rb) == 1);
    assert(rb_at(rb) == 4);

    rb_push(rb, 'o');
    rb_push(rb, 'n'); // [o, n, , , s,]

    n = rb_read(rb, bytes, 3);
    assert(!strncmp("son", bytes, 3));
    assert(n == 3);
    assert(rb_size(rb) == 0);
    assert(rb_at(rb) == 2);

    assert(rb_write(rb, "hel", 3) == 3);
    assert(rb_at(rb) == 2);
    assert(rb_write(rb, "o world!", 9) == 2);
    memset(bytes, '\0', sizeof(bytes));
    assert(rb_read(rb, bytes, 10) == 5);
    assert(!memcmp((char[]){'h', 'e', 'l', 'o', ' '}, bytes, 5));
    assert(rb_size(rb) == 0);
    assert(rb_at(rb) == 2);

    rb_free(rb);

    return 0;
}
