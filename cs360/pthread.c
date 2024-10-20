#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

typedef struct Work {
  pthread_cond_t *condvar;
  pthread_mutex_t *mutex;
  int prime_to_test;
  bool result;
} Work;

int i;

static void *worker(void *arg) {
  Work *w = (Work *)arg;

  pthread_mutex_lock(w->mutex);
  pthread_cond_wait(w->condvar, w->mutex);
  w->result = true;
  for (int i = 2; i < sqrt(w->prime_to_test); i++) {
    if (w->prime_to_test % i == 0) {
      w->result = false;
      break;
    }
  }

  pthread_mutex_unlock(w->mutex);

  return w;
}

int main() {
  pthread_t tid[10];
  Work work[10];

  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;

  for (int i = 0; i < 10; i++) {
    work[i].condvar = &condvar;
    work[i].mutex = &mutex;
    work[i].prime_to_test = i + 2;

    pthread_create(tid + i, NULL, worker, work + i);
    pthread_cond_signal(&condvar);
  }

  sleep(1);

  for (int i = 0; i < 10; i++) {
  }

  Work *j;
  for (int i = 0; i < 10; i++) {
    pthread_join(tid[i], (void **)&j);

    printf("Result for %d is %d\n", j->prime_to_test, j->result);
  }

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condvar);
}
