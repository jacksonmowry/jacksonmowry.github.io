#include <stdio.h>

#define BUFFER_SIZE 1024

typedef struct file_buffer {
  FILE fp;
  size_t cursor;
  char buffer[BUFFER_SIZE];
} file_buffer;

int main() {}
