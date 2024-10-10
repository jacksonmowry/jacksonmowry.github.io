#ifndef BUFREAD_H_
#define BUFREAD_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

struct BufReader;
typedef struct BufReader BufReader;

BufReader* br_open(const char path[], size_t capacity, size_t fill_trigger);
void br_close(BufReader* br);
void br_seek(BufReader* br, ssize_t offset, int whence);
ssize_t br_tell(const BufReader* br);
bool br_getline(BufReader* br, char s[], size_t size);
int br_getchar(BufReader* br);
size_t br_read(BufReader* br, void* dest, size_t max_bytes);

#endif // BUFREAD_H_
