#include "lab6/include/tpool.h"
#include <assert.h>
#include <dirent.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct Work {
    char* filename;
    bool large_hash;
    size_t result_index;
} Work_t;

typedef struct TPool {
    pthread_t* tids;
    size_t num_tids;

    bool die;

    Work_t** buf;
    size_t ring_size;
    size_t ring_cap;
    size_t ring_at;

    uint64_t* results;
    size_t results_len;
    size_t results_cap;

    int outstanding_work;

    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    pthread_cond_t all_done;
    pthread_mutex_t mutex;
} TPool_t;

void* worker(void* arg) {
    TPool_t* tp = (TPool_t*)arg;

    while (true) {
        pthread_mutex_lock(&tp->mutex);
        while (tp->ring_size == 0 && !tp->die) {
            pthread_cond_wait(&tp->not_empty, &tp->mutex);

            if (tp->die) {
                pthread_mutex_unlock(&tp->mutex);
                return NULL;
            }
        }

        Work_t* w = tp->buf[tp->ring_at % tp->ring_cap];
        tp->ring_size--;
        tp->ring_at++;

        pthread_mutex_unlock(&tp->mutex);
        pthread_cond_signal(&tp->not_full);

        // simulate work
        uint64_t result = rand();

        pthread_mutex_lock(&tp->mutex);

        if (w->result_index >= tp->results_cap) {
            tp->results_cap *= 2;
            tp->results =
                realloc(tp->results, sizeof(*tp->results) * tp->results_cap);
        }

        tp->results[w->result_index] = result;
        tp->results_len++;
        tp->outstanding_work--;

        pthread_cond_signal(&tp->all_done);
        pthread_mutex_unlock(&tp->mutex);

        free(w);
    }

    pthread_mutex_unlock(&tp->mutex);
    return NULL;
}

TPool_t* init(size_t num_threads) {
    if (num_threads == 0 || num_threads > 32) {
        return NULL;
    }

    TPool_t* tp = calloc(1, sizeof(*tp));
    *tp = (TPool_t){.tids = malloc(num_threads * sizeof(*tp->tids)),
                    .num_tids = num_threads,
                    .buf = calloc(num_threads, sizeof(*tp->buf)),
                    .ring_cap = num_threads,
                    .results = calloc(num_threads, sizeof(*tp->results)),
                    .results_cap = num_threads,
                    .not_full = PTHREAD_COND_INITIALIZER,
                    .not_empty = PTHREAD_COND_INITIALIZER,
                    .all_done = PTHREAD_COND_INITIALIZER,
                    .mutex = PTHREAD_MUTEX_INITIALIZER};

    for (int i = 0; i < num_threads; i++) {
        pthread_create(tp->tids + i, NULL, worker, tp);
    }

    return tp;
}

void wrapper(TPool_t* tp, size_t num_threads) {
    tp->results_len = 0;
    tp->outstanding_work = 0;

    for (int i = 0; i < 32; i++) {
        Work_t* w = malloc(sizeof(*w));
        w->result_index = i;
        pthread_mutex_lock(&tp->mutex);
        while (tp->ring_size == tp->ring_cap) {
            pthread_cond_wait(&tp->not_full, &tp->mutex);
        }

        tp->buf[(tp->ring_at + tp->ring_size) % tp->ring_cap] = w;
        tp->ring_size++;
        pthread_mutex_unlock(&tp->mutex);
        pthread_cond_signal(&tp->not_empty);
    }

    pthread_mutex_lock(&tp->mutex);
    while (tp->outstanding_work > 0) {
        pthread_cond_wait(&tp->all_done, &tp->mutex);
    }
    pthread_mutex_unlock(&tp->mutex);

    for (int i = 0; i < tp->results_len; i++) {
        printf("%lu\n", tp->results[i]);
    }
}

void cleanup(TPool_t* tp) {
    pthread_mutex_lock(&tp->mutex);
    tp->die = true;
    pthread_cond_broadcast(&tp->not_empty);
    pthread_mutex_unlock(&tp->mutex);

    for (int i = 0; i < tp->num_tids; i++) {
        pthread_join(tp->tids[i], NULL);
    }

    free(tp->tids);
    free(tp->buf);
    free(tp->results);

    pthread_cond_destroy(&tp->not_empty);
    pthread_cond_destroy(&tp->not_full);
    pthread_cond_destroy(&tp->all_done);
    pthread_mutex_destroy(&tp->mutex);
}

int main() {
    TPool_t* tp = init(8);

    wrapper(tp, 32);

    cleanup(tp);
}
