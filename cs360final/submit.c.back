// Jackson Mowry

#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

unsigned int yolosum(unsigned int value);

static bool is_prime(unsigned int val) {
  bool prime = true;
  for (int i = 2; i <= sqrt(val); i++) {
    if (val % i == 0) {
      prime = false;
    }
  }

  return prime;
}

typedef struct Work {
  unsigned int val;
  pthread_mutex_t mutex;
  size_t total_primes;
  unsigned int upper_bound;
} Work_t;

void *worker(void *arg) {
  Work_t *w = (Work_t *)arg;

  while (true) {
    pthread_mutex_lock(&w->mutex);

    unsigned int val = w->val;
    w->val++;

    if (val > w->upper_bound) {
      pthread_mutex_unlock(&w->mutex);
      return NULL;
    }

    pthread_mutex_unlock(&w->mutex);

    unsigned int result = yolosum(val);

    if (is_prime(result)) {
      pthread_mutex_lock(&w->mutex);
      w->total_primes += 1;
      pthread_mutex_unlock(&w->mutex);
    }
  }
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

  Work_t w = {.mutex = PTHREAD_MUTEX_INITIALIZER,
              .total_primes = 0,
              .upper_bound = sv,
              .val = fv};

  pthread_t tids[num_threads];
  for (int i = 0; i < num_threads; i++) {
    pthread_create(tids + i, NULL, worker, &w);
  }

  for (int i = 0; i < num_threads; i++) {
    pthread_join(tids[i], NULL);
  }

  pthread_mutex_destroy(&w.mutex);

  printf("%zu\n", w.total_primes);
}
