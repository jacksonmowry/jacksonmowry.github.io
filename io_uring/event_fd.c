#include <fcntl.h>
#include <liburing.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

#define BUFF_SZ 512

char buff[BUFF_SZ + 1];
struct io_uring ring;
void error_exit(char *message) {
  perror(message);
  exit(EXIT_FAILURE);
}

void *listener_thread(void *data) {
  struct io_uring_cqe *cqe;
  int event_file_descriptor = (int)data;
  eventfd_t v;
  printf("%s: jwaiting for completion event...\n", __FUNCTION__);

  int ret = eventfd_read(event_file_descriptor, &v);
  if (ret < 0)
    error_exit("eventfd_read");

  printf("%s: Got completion event.\n", __FUNCTION__);

  ret = io_uring_wait_cqe(&ring, &cqe);
  if (ret < 0) {
    fprintf(stderr, "Error in async operation: %s\n", strerror(-cqe->res));
  }
  printf("Result of the operation: %d\n", cqe->res);
  io_uring_cqe_seen(&ring, cqe);

  printf("Contents read from file:\n%s\n", buff);
  return NULL;
}

int setup_io_uring(int efd) {
  int ret = io_uring_queue_init(8, &ring, 0);
  if (ret) {
    fprintf(stderr, "Unable to setup io_uring: %s\n", strerror(-ret));
    return 1;
  }
  io_uring_register_eventfd(&ring, efd);
  return 0;
}

int read_file_with_io_uring() {
  struct io_uring_sqe *sqe;
  sqe = io_uring_get_sqe(&ring);
  if (!sqe) {
    fprintf(stderr, "Could not get SQE.\n");
    return 1;
  }

  int fd = open("etc/passwd", O_RDONLY);
  io_uring_prep_read(sqe, fd, buff, BUFF_SZ, 0);
  io_uring_submit(&ring);

  return 0;
}

int main() {
  pthread_t t;
  int efd;

  efd = eventfd(0, 0);
  if (efd < 0) {
    error_exit("eventfd");
  }

  pthread_create(&t, NULL, listener_thread, (void *)efd);

  sleep(2);

  setup_io_uring(efd);

  read_file_with_io_uring();

  pthread_join(t, NULL);

  io_uring_queue_exit(&ring);
  return EXIT_SUCCESS;
}
