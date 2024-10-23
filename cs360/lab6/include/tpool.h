#ifndef TPOOL_H_
#define TPOOL_H_

// Thread pool functions
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void* thread_pool_init(int num_threads);
bool thread_pool_hash(void* handle, const char* directory, int hash_size);
void thread_pool_shutdown(void* handle);

// HASHING FUNCTIONS
uint32_t hash32(FILE* fl);
uint64_t hash64(FILE* fl);

#endif // TPOOL_H_
