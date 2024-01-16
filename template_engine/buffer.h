#ifndef BUFFER_H_
#define BUFFER_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct buffer {
  char *buffer;
  size_t len;
  size_t cap;
} buffer;

void buffer_resize(buffer *b, size_t added_size) {
  if (b->len + added_size > b->cap) {
    b->cap += MAX(4096, added_size);
    b->buffer = realloc(b->buffer, b->cap);
    if (!b->buffer) {
      perror("realloc");
      exit(1);
    }
  }
}

void buffer_write(buffer *b, const char *format, const char *argument) {
  int length = snprintf(NULL, 0, format, argument);
  buffer_resize(b, length);
  b->len += sprintf(&b->buffer[b->len], format, argument);
  b->buffer[b->len] = '\0';
}

void buffer_write_var(buffer *b, const char *format, const char *new_fmt,
                      const char *variable) {
  int length = snprintf(NULL, 0, format, new_fmt, variable);
  buffer_resize(b, length);
  b->len += sprintf(&b->buffer[b->len], format, new_fmt, variable);
  b->buffer[b->len] = '\0';
}

void buffer_strcpy(buffer *b, const char *string) {
  size_t size = strlen(string);
  buffer_resize(b, size);
  strncpy(&b->buffer[b->len], string, size);
  b->len += size;
  b->buffer[b->len] = '\0';
}

void buffer_append(buffer *b, buffer *other) {
  buffer_resize(b, other->len);
  strcpy(&b->buffer[b->len], other->buffer);
  b->len += other->len;
  b->buffer[b->len] = '\0';
}


#endif // BUFFER_H_















































