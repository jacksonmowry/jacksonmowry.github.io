#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    unsigned int values;
    bool done;
    bool result;
} Task;

typedef struct {
    Task** buffer;

    unsigned int size;
    unsigned int at;
    unsigned int capacity;

    bool die;

    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    pthread_mutex_t mutex;
} Queue;

static void* worker(void* arg);

int main(int argc, char* argv[]) {
    Queue queue;
    unsigned int queue_size;
    unsigned int vector_size;
    unsigned int num_thread;
    unsigned int start_value;
    unsigned int end_value;

    if (argc != 6) {
        fprintf(stderr,
                "Usage: %s <queue size> <vector_size> <num threads> <start> "
                "<end>\n",
                argv[0]);
        return -1;
    }

    if (sscanf(argv[1], "%u", &queue_size) != 1) {
        printf("Invalid queue_size '%s'\n", argv[1]);
    }
    if (sscanf(argv[2], "%u", &vector_size) != 1) {
        printf("Invalid vector size '%s'\n", argv[2]);
    }
    if (sscanf(argv[3], "%u", &num_thread) != 1) {
        printf("Invalid number of threads '%s'\n", argv[3]);
    }
    if (sscanf(argv[4], "%u", &start_value) != 1) {
        printf("Invalid start '%s'\n", argv[4]);
    }
    if (sscanf(argv[5], "%u", &end_value) != 1) {
        printf("Invalid end '%s'\n", argv[5]);
    }

    queue.capacity = queue_size;
    queue.size = 0;
    queue.at = 0;
    queue.buffer = calloc(queue_size, sizeof(Task*));
    queue.die = false;

    pthread_cond_init(&queue.not_empty, NULL);
    pthread_cond_init(&queue.not_full, NULL);
    pthread_mutex_init(&queue.mutex, NULL);

    pthread_t tids[num_thread];
    Task tasks[vector_size];

    for (int i = 0; i < num_thread; i++) {
        pthread_create(tids + i, NULL, worker, &queue);
    }

    while (start_value <= end_value) {
        for (int i = 0; i < vector_size && start_value <= end_value; i++) {
            tasks[i].values = start_value++;
            tasks[i].done = false;

            pthread_mutex_lock(&queue.mutex);

            while (queue.size == queue.capacity) {
                pthread_cond_wait(&queue.not_full, &queue.mutex);
            }

            queue.buffer[(queue.size + queue.at) % queue.capacity] = tasks + i;
            queue.size += 1;

            pthread_cond_signal(&queue.not_empty);
            pthread_mutex_unlock(&queue.mutex);
        }

        for (int i = 0; i < vector_size; i++) {
            while (!tasks[i].done) {
                usleep(100);
            }

            printf("%d is %sprime\n", tasks[i].values,
                   tasks[i].result ? "" : " not");
        }
    }

    while (queue.size > 0) {
        usleep(100);
    }

    queue.die = true;
    pthread_cond_broadcast(&queue.not_empty);

    for (int i = 0; i < num_thread; i++) {
        pthread_join(tids[i], NULL);
    }

    pthread_cond_destroy(&queue.not_empty);
    pthread_cond_destroy(&queue.not_full);
    pthread_mutex_destroy(&queue.mutex);

    free(queue.buffer);
}

static void* worker(void* arg) {
    Queue* q = (Queue*)arg;

    while (true) {
        pthread_mutex_lock(&q->mutex);

        while (q->size == 0 && !q->die) {
            pthread_cond_wait(&q->not_empty, &q->mutex);
        }

        if (q->die) {
            pthread_mutex_unlock(&q->mutex);
            return NULL;
        }

        Task* t = q->buffer[q->at];
        q->at = (q->at + 1) % q->capacity;
        q->size -= 1;

        pthread_cond_signal(&q->not_full);
        pthread_mutex_unlock(&q->mutex);

        bool is_prime = true;
        for (int i = 2; i < sqrt(t->values); i++) {
            if ((t->values % i) == 0) {
                is_prime = false;
                break;
            }
        }

        t->done = true;
        t->result = is_prime;
    }

    return NULL;
}
