#include <assert.h>
#include <dirent.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

unsigned int yolosum(unsigned int value);

typedef struct Work {
  unsigned int value;
} Work_t;

typedef struct TPool {
  pthread_t *tids;
  size_t num_tids;

  bool die;

  Work_t **buf;
  size_t ring_size;
  size_t ring_cap;
  size_t ring_at;

  size_t num_prime;

  int outstanding_work;

  pthread_cond_t not_full;
  pthread_cond_t not_empty;
  pthread_cond_t all_done;
  pthread_mutex_t mutex;
} TPool_t;

void *worker(void *arg) {
  TPool_t *tp = (TPool_t *)arg;

  while (true) {
    pthread_mutex_lock(&tp->mutex);
    while (tp->ring_size == 0 && !tp->die) {
      pthread_cond_wait(&tp->not_empty, &tp->mutex);

      if (tp->die) {
        pthread_mutex_unlock(&tp->mutex);
        return NULL;
      }
    }

    Work_t *w = tp->buf[tp->ring_at % tp->ring_cap];
    tp->ring_at++;
    tp->ring_size--;

    pthread_cond_signal(&tp->not_full);
    pthread_mutex_unlock(&tp->mutex);

    // Do work!
    bool result = rand() % 2;
    pthread_mutex_lock(&tp->mutex);
    tp->num_prime += result;
    tp->outstanding_work--;
    pthread_cond_signal(&tp->all_done);
    pthread_mutex_unlock(&tp->mutex);

    free(w);
  }

  pthread_mutex_unlock(&tp->mutex);
  return NULL;
}

TPool_t *thread_pool_init(size_t num_threads) {
  TPool_t *tp = malloc(sizeof(*tp));
  *tp = (TPool_t){.tids = malloc(num_threads * sizeof(*tp->tids)),
                  .num_tids = num_threads,
                  .buf = calloc(num_threads, sizeof(*tp->buf)),
                  .ring_cap = num_threads,
                  .not_full = PTHREAD_COND_INITIALIZER,
                  .not_empty = PTHREAD_COND_INITIALIZER,
                  .all_done = PTHREAD_COND_INITIALIZER,
                  .mutex = PTHREAD_MUTEX_INITIALIZER};

  for (int i = 0; i < num_threads; i++) {
    pthread_create(tp->tids + i, NULL, worker, tp);
  }

  return tp;
}

void cleanup(TPool_t *tp) {
  pthread_mutex_lock(&tp->mutex);
  tp->die = true;
  pthread_mutex_unlock(&tp->mutex);
  pthread_cond_broadcast(&tp->not_empty);

  for (int i = 0; i < tp->num_tids; i++) {
    pthread_join(tp->tids[i], NULL);
  }

  free(tp->tids);
  free(tp->buf);

  pthread_cond_destroy(&tp->not_full);
  pthread_cond_destroy(&tp->not_empty);
  pthread_cond_destroy(&tp->all_done);
  pthread_mutex_destroy(&tp->mutex);
}

void wrapper(TPool_t *tp, size_t lower, size_t upper) {
  for (int i = lower; i <= upper; i++) {
    Work_t *w = malloc(sizeof(*w));
    w->value = i;

    pthread_mutex_lock(&tp->mutex);
    while (tp->ring_size == tp->ring_cap) {
      pthread_cond_wait(&tp->not_full, &tp->mutex);
    }

    tp->buf[(tp->ring_at + tp->ring_size) % tp->ring_cap] = w;
    tp->ring_size++;

    pthread_cond_signal(&tp->not_empty);
    pthread_mutex_unlock(&tp->mutex);
  }

  pthread_mutex_lock(&tp->mutex);
  while (tp->outstanding_work > 0) {
    pthread_cond_wait(&tp->all_done, &tp->mutex);
  }
  printf("%zu\n", tp->num_prime);

  pthread_mutex_unlock(&tp->mutex);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s <num threads> <start> <end>\n", argv[0]);
    return -1;
  }

  size_t num_threads;
  size_t fv;
  size_t sv;

  if (sscanf(argv[1], "%lu", &num_threads) != 1) {
    printf("Invalid number of threads: %s\n", argv[1]);
    return -1;
  }
  if (sscanf(argv[2], "%lu", &fv) != 1) {
    printf("Invalid start: %s\n", argv[2]);
    return -1;
  }
  if (sscanf(argv[3], "%lu", &sv) != 1) {
    printf("Invalid end: %s\n", argv[3]);
    return -1;
  }

  if (num_threads == 0 || num_threads > 100) {
    printf("Invalid number of threads: %lu\n", num_threads);
    return -1;
  }
  if (fv < 2) {
    printf("Invalid start: %lu\n", fv);
    return -1;
  }
  if (sv < fv) {
    printf("Invalid end: %lu\n", sv);
    return -1;
  }

  TPool_t *tp = thread_pool_init(num_threads);
  wrapper(tp, fv, sv);
  puts("Cleaning up!");
  cleanup(tp);

  puts("All done!");
}
