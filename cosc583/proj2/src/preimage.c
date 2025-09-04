#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha1.h"

size_t truncation_length;

typedef struct HashResult {
    uint8_t hash[SHA1_BLOCK_SIZE];
} HashResult;

HashResult hash(uint8_t *data, size_t data_length) {
    HashResult hr = {};
    SHA1(data, data_length, hr.hash);

    return hr;
}

bool hash_compare(HashResult *a, HashResult *b, size_t truncation_length) {
    size_t i = 0;
    while (truncation_length >= 8) {
        if (a->hash[i] != b->hash[i]) {
            return false;
        }

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

    return (a->hash[i] & mask) == (b->hash[i] & mask);
}

int find_preimage_collision(size_t truncation_length) {
    FILE *f = fopen("/dev/urandom", "r");
    uint8_t buffer[20] = {0};
    fread(buffer, sizeof(buffer), 1, f);

    HashResult hr = hash(buffer, sizeof(buffer));

    fprintf(stderr, "Attempting to find a preimage collision for ");
    for (size_t i = 0; i < 20; i++) {
        fprintf(stderr, "%02x", hr.hash[i]);
    }
    fprintf(stderr, "\n");

    for (size_t i = 1; i > 0; i++) {
        uint8_t inner_buffer[20] = {0};
        fread(inner_buffer, sizeof(inner_buffer), 1, f);

        HashResult new_hr = hash(inner_buffer, sizeof(inner_buffer));

        if (hash_compare(&hr, &new_hr, truncation_length)) {
            fprintf(stderr, "Found a collision with hash ");
            for (size_t i = 0; i < 20; i++) {
                fprintf(stderr, "%02x", new_hr.hash[i]);
            }
            fprintf(stderr, "\n");
            fclose(f);
            return i;
        }
    }

    fclose(f);
    return -1;
}

bool running = true;
atomic_size_t runs = 0;

void *worker(void *arg) {
    FILE *f = fopen("/dev/urandom", "r");
    uint8_t buffer[20] = {0};
    HashResult *target_hr = (HashResult *)arg;

    while (running) {
        size_t run = runs++;
        fread(buffer, sizeof(buffer), 1, f);
        HashResult hr = hash(buffer, sizeof(buffer));

        if (hash_compare(target_hr, &hr, truncation_length)) {
            running = false;

            printf("Found collision on run %zu\n", run);
            fprintf(stderr, "Found a collision with hash ");
            for (size_t i = 0; i < 20; i++) {
                fprintf(stderr, "%02x", hr.hash[i]);
            }
            fprintf(stderr, "\n");
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s threads truncation_length_bits \n", argv[0]);
        exit(1);
    }

    size_t threads = strtoull(argv[1], NULL, 10);
    truncation_length = strtoull(argv[2], NULL, 10);

    FILE *f = fopen("/dev/urandom", "r");
    uint8_t buffer[20] = {0};
    fread(buffer, sizeof(buffer), 1, f);
    fclose(f);

    HashResult hr = hash(buffer, sizeof(buffer));
    fprintf(stderr, "Attempting to find a preimage collision for ");
    for (size_t i = 0; i < 20; i++) {
        fprintf(stderr, "%02x", hr.hash[i]);
    }
    fprintf(stderr, "\n");

    pthread_t thread_pool[threads];
    for (size_t i = 0; i < threads; i++) {
        pthread_create(thread_pool + i, NULL, worker, &hr);
    }

    for (size_t i = 0; i < threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    /* int result = find_preimage_collision(truncation_length); */

    /* printf("I found a preimage collision in %d tries\n", result); */
}
