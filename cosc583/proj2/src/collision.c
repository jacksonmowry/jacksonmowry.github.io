#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha1.h"

size_t truncation_length;
uint32_t *hashes;
atomic_size_t hashes_len = 0;
const size_t hashes_cap = 1 << 28;

bool running = true;
atomic_size_t runs = 0;

typedef struct HashResult {
    uint8_t hash[SHA1_BLOCK_SIZE];
} HashResult;

HashResult hash(uint8_t *data, size_t data_length) {
    HashResult hr = {};
    SHA1(data, data_length, hr.hash);

    return hr;
}

uint32_t truncate(HashResult *hash, size_t truncation_length) {
    uint32_t result = 0;
    size_t i = 0;
    while (truncation_length >= 8) {
        result |= ((uint32_t)hash->hash[i]) << (24 - (8 * i));

        i++;
        truncation_length -= 8;
    }

    uint8_t mask = 0;
    switch (truncation_length) {
    case 7:
        mask |= 1 << 1;
    case 6:
        mask |= 1 << 2;
    case 5:
        mask |= 1 << 3;
    case 4:
        mask |= 1 << 4;
    case 3:
        mask |= 1 << 5;
    case 2:
        mask |= 1 << 6;
    case 1:
        mask |= 1 << 7;
    }

    result |= (hash->hash[i] & mask) << (24 - (8 * i));

    return result;
}

void *worker(void *arg) {
    FILE *f = fopen("/dev/urandom", "r");
    uint8_t buffer[20] = {0};

    while (running) {
        size_t run = runs++;
        fread(buffer, sizeof(buffer), 1, f);
        HashResult hr = hash(buffer, sizeof(buffer));

        size_t my_index = hashes_len++;
        if (my_index >= hashes_cap) {
            fprintf(stderr, "we ran out of space\n");
            exit(1);
        }
        uint32_t truncated_result = truncate(&hr, truncation_length);
        hashes[my_index] = truncated_result;

        for (size_t i = 0; i < my_index; i++) {
            if (hashes[i] == truncated_result) {
                running = false;

                printf("Found collision on run %zu\n", run);
                fprintf(stderr, "Found a collision with hash ");
                for (size_t i = 0; i < 20; i++) {
                    fprintf(stderr, "%02x", hr.hash[i]);
                }
                fprintf(stderr, "\n");

                fprintf(stderr, "Stored hash: %08x == Calculated hash: %08x\n",
                        hashes[i], truncated_result);
            }
        }
    }

    fclose(f);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s threads truncation_length_bits \n", argv[0]);
        exit(1);
    }

    size_t threads = strtoull(argv[1], NULL, 10);
    truncation_length = strtoull(argv[2], NULL, 10);

    hashes = malloc(sizeof(uint32_t) * hashes_cap);

    pthread_t thread_pool[threads];
    for (size_t i = 0; i < threads; i++) {
        pthread_create(thread_pool + i, NULL, worker, NULL);
    }

    for (size_t i = 0; i < threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }
}
