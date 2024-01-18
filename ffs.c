#include <alloca.h>
#include <assert.h>
#include <bits/pthreadtypes.h>
#include <dirent.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdarg.h>

#define CYAN_BOLD "\033[1;36m"
#define RED_BOLD "\033[1;31m"
#define RESET "\033[0m"

pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;
bool terminate_work = false;

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct buffer {
  char *buffer;
  size_t len;
  size_t cap;
} buffer;

void buffer_resize(buffer *b, size_t added_size) {
  if (b->len + added_size > b->cap) {
    b->cap += MAX(4096, added_size);
    b->buffer = realloc(b->buffer, b->cap);
    if (!b->buffer) {
      perror("realloc");
      exit(1);
    }
  }
}

void buffer_write(buffer *b, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int length = vsnprintf(NULL, 0, format, args) + 1;
  va_end(args);
  va_start(args, format);
  buffer_resize(b, length );
  b->len += vsprintf(&b->buffer[b->len], format, args);
  b->buffer[b->len] = '\0';
  va_end(args);
}

struct node {
  struct node *next;
  char *name;
};
typedef struct node node_t;

node_t *head = NULL;
node_t *tail = NULL;

void enqueue(char *entry) {
  node_t *newnode = malloc(sizeof(node_t));
  newnode->name = entry;
  newnode->next = NULL;
  if (tail == NULL) {
    head = newnode;
  } else {
    tail->next = newnode;
  }
  tail = newnode;
}

// dequeue returns dirent* or NULL
char *dequeue(void) {
  if (head == NULL) {
    return NULL;
  } else {
    char *result = head->name;
    node_t *temp = head;
    head = head->next;
    if (head == NULL) {
      tail = NULL;
    }
    free(temp);
    return result;
  }
}

// Recursively scans a directory placing all files
// on the work queue
void handle_dir(const char *dir_path) {
  DIR *dir;
  struct dirent *entry;

  dir = opendir(dir_path);
  if (!dir) {
    perror("opendir");
    exit(1);
  }

  while ((entry = readdir(dir))) {
    char path[PATH_MAX];
    sprintf(path, "%s/%s", dir_path, entry->d_name);
    if (entry->d_type == DT_REG) {
      pthread_mutex_lock(&q_mutex);
      enqueue(strdup(path));
      pthread_mutex_unlock(&q_mutex);
    } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      handle_dir(path);
    }
  }

  closedir(dir);
}

void file_search(const char *const filename, const char *const needle) {
  buffer b = (buffer){.buffer = malloc(128), .len = 0, .cap = 128};

  buffer_write(&b, "%s%s%s:\n", CYAN_BOLD, filename, RESET);

  FILE *fp = fopen(filename, "r");
  if (!fp) {
    perror("fopen");
    return;
  }

  char *buf = NULL;
  size_t len;
  size_t read;
  size_t line_number = 0;
  bool found_something = NULL;

  while ((read = getline(&buf, &len, fp)) != -1) {
    line_number++;
    char *prev = buf;
    char *found = strcasestr(buf, needle);
    if (found) {
      buffer_write(&b, "%s%lu%s: ", CYAN_BOLD, line_number, RESET);
    }
    while (found) {
      found_something = true;
      size_t len_before = found - prev;
      buffer_write(&b, "%.*s", (int)len_before, prev);
      buffer_write(&b, "%s%.*s%s", RED_BOLD, (int)strlen(needle), found, RESET);
      prev = found + strlen(needle);
      found = strcasestr(prev, needle);
      if (!found) {
        buffer_write(&b, "%s", prev);
      }
    }
  }

  if (found_something) {
    pthread_mutex_lock(&io_mutex);
    printf("%s\n", b.buffer);
    pthread_mutex_unlock(&io_mutex);
  }

  free(b.buffer);
  free(buf);
  return;
}

void stdin_search(const char *const needle) {
  char *buf = NULL;
  size_t len;
  size_t read;

  while ((read = getline(&buf, &len, stdin)) != -1) {
    char *prev = buf;
    char *found = strcasestr(buf, needle);
    while (found) {
      size_t len_before = found - prev;
      printf("%.*s", (int)len_before, prev);
      printf("%s%.*s%s", RED_BOLD, (int)strlen(needle), found, RESET);
      prev = found + strlen(needle);
      found = strcasestr(prev, needle);
      if (!found) {
        printf("%s", prev);
      }
    }
  }
  free(buf);
  return;
}

void *thread_function(void *arg) {
  char *needle = (char*)arg;
  while (true) {
    char *filename;
    pthread_mutex_lock(&q_mutex);
    if (!terminate_work && head == NULL) {
      pthread_mutex_unlock(&q_mutex);
      continue;
    }
    if (terminate_work && head == NULL) {
      pthread_mutex_unlock(&q_mutex);
      break;
    }

    filename = dequeue();

    pthread_mutex_unlock(&q_mutex);
    if (filename) {
      file_search(filename, needle);
      free(filename);
    }
  }
  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: %s needle files...", argv[0]);
    exit(1);
  }

  char *needle = argv[1];

  if (!isatty(fileno(stdin))) {
    // pipe
    stdin_search(needle);
    exit(0);
  }

  long num_cpu = sysconf(_SC_NPROCESSORS_CONF);
  if (num_cpu <= 0) {
    fprintf(stderr, "Error determining the number of CPUs.\n");
    exit(1);
  }

  pthread_t *thread_pool = alloca(num_cpu * sizeof(pthread_t));
  for (int i = 0; i < num_cpu; i++) {
    pthread_create(&thread_pool[i], NULL, thread_function, (void*)needle);
  }

  handle_dir(".");
  terminate_work = true;

  for (int i = 0; i < num_cpu; i++) {
    pthread_join(thread_pool[i], NULL);
  }

}
