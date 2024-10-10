#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <linux/sched.h>
#include <sched.h>

#define NUM_ITERATIONS 1000000

int clone(int (*fn)(void *), void *stack, int flags, void *args, ...);

void child() {
  int *i;
  int fd;
  sleep(2);
  fd = shm_open("/myshared.mem", O_RDWR, 0);

  i = mmap(NULL, sizeof(*i), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
  if (i == MAP_FAILED) {
    perror("mmap");
    return;
  }

  for (int loop = 0; loop < NUM_ITERATIONS; loop++) {
    *i = *i + 1;
  }

  munmap(i, sizeof(*i));
}

void parent() {
  int *i;
  int fd;
  fd = shm_open("/myshared.mem", O_CREAT | O_EXCL | O_RDWR, 0666);

  if (shm_open < 0) {
    perror("shm_open");
    return;
  }

  ftruncate(fd, sizeof(*i));
  i = mmap(NULL, sizeof(*i), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
  if (i == MAP_FAILED) {
    perror("mmap");
    return;
  }

  for (int loop = 0; loop < NUM_ITERATIONS; loop++) {
    *i = *i + 1;
  }

  sleep(2);
  printf("i = %d\n", *i);

  munmap(i, sizeof(*i));
  shm_unlink("/myshared.mem");
}

int main(void) {
  pid_t pid;
  sem_t sem;

  sem_init(&sem, 1, 4);

  pid = fork();

  if (pid == 0) {
    child();
  } else if (pid > 0) {
    parent();
  }

  sem_destroy(&sem);
}
