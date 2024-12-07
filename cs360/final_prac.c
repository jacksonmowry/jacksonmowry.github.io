#include <dirent.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Work {
    char* needle;
    struct dirent entry;
    struct Work* next;
} Work_t;

Work_t* head = NULL;

void add_work(Work_t w) {
    Work_t* new = malloc(sizeof(Work_t));
    *new = w;

    new->next = head;
    head = new;
}

Work_t* get_work(void) {
    Work_t* ret = head;

    if (head) {
        head = head->next;
    }

    return ret;
}

void* worker(void* arg) {
    while (true) {
        pthread_mutex_lock(&mutex);
        Work_t* w = get_work();
        pthread_mutex_unlock(&mutex);

        if (!w) {
            break;
        }

        FILE* fp = fopen(w->entry.d_name, "r");

        char* line = NULL;
        size_t len = 0;
        ssize_t nread;

        while ((nread = getline(&line, &len, fp)) != -1) {
            if (strstr(line, w->needle)) {
                pthread_mutex_lock(&mutex);
                printf("%s: %s", w->entry.d_name, line);
                pthread_mutex_unlock(&mutex);
            }
        }

        fclose(fp);
        free(line);
        free(w);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    int flags, opt;
    int threads;
    char* needle = "";

    flags = 0;
    while ((opt = getopt(argc, argv, "n:t:")) != -1) {
        switch (opt) {
        case 't': {
            if (sscanf(optarg, "%d", &threads) != 1) {
                fprintf(stderr, "usage: %s <num_threads>\n", argv[0]);
            }
        } break;
        case 'n':
            needle = optarg;
            break;
        default:
            fprintf(stderr, "usage: %s <num_threads>\n", argv[0]);
            return 1;
        }
    }

    DIR* d = opendir(".");
    struct dirent* entry;

    while ((entry = readdir(d))) {
        add_work((Work_t){.entry = *entry, .needle = needle});
    }

    closedir(d);

    pthread_t pids[threads];
    for (int i = 0; i < threads; i++) {
        pthread_create(pids + i, NULL, worker, NULL);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(pids[i], NULL);
    }
}
