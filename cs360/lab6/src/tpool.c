#include "tpool.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct TPool {
    pthread_t* tids;
    union {
        uint32_t small_hash;
        uint64_t big_hash;
    }* hash;
    size_t num_files;
    __atomic int outstanding_jobs;
} TPool;

void* worker(void* arg) { return NULL; }

uint32_t hash32(FILE* fl) {
    uint32_t digest = 2166136261;

    uint8_t byte;
    while (fread(&byte, 1, 1, fl)) {
        digest = (digest ^ byte) * 16777619;
    }

    return digest;
}

uint64_t hash64(FILE* fl) {
    uint64_t digest = 14695981039346656037UL;

    uint8_t byte;
    while (fread(&byte, 1, 1, fl)) {
        digest = (digest ^ byte) * 1099511628211;
    }

    return digest;
}

void* thread_pool_init(int num_threads) {
    TPool* tp = calloc(1, sizeof(TPool));

    tp->tids = calloc(num_threads, sizeof(pthread_t));

    for (int i = 0; i < num_threads; i++) {
        pthread_create(tp->tids + i, NULL, worker, NULL);
    }

    return tp;
}

int main() {
    FILE* fp = fopen("test2.txt", "r");

    printf("hash: %x\n", hash32(fp));

    return 0;
}
