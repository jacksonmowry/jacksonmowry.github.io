#include "hashmap/hashmap.h"
#include <assert.h>
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <threads.h>
#include <unistd.h>

int parse_int(const char* c) {
    bool negative = (*c == '-');
    c += negative;

    char num[4] = {0};
    int num_pos = 0;
    while (*c != '\0') {
        num[num_pos] = (*c != '.')
                           ? *c
                           : num[num_pos]; // If c is '.', keep num[num_pos] the
                                           // same, otherwise assign *c to it
        num_pos += (*c != '.'); // Increment num_pos only if c is not '.'
        c++;
    }

    int accum = 0;
    char* cursor = num;
    while (*cursor) {
        accum = (accum * 10) + (*cursor - '0');
        cursor++;
    }

    // 2's complement by hand
    return (accum ^ -negative) + negative;
}

// Comparison function for qsort
int compare_pairs(const void* a, const void* b) {
    const pair* pair_a = (const pair*)a;
    const pair* pair_b = (const pair*)b;

    // Assuming key is a string (char *)
    const char* key_a = (const char*)pair_a->key;
    const char* key_b = (const char*)pair_b->key;

    if (key_a == NULL && key_b == NULL) {
        return 0; // Both keys are null, consider them equal
    } else if (key_a == NULL) {
        return 1; // Null key should come after non-null key
    } else if (key_b == NULL) {
        return -1; // Null key should come after non-null key
    } else {
        return strcmp(key_a, key_b); // Compare non-null keys
    }
}

hashmap h;
mtx_t hashmap_mutex;
pthread_rwlock_t rw_lock;

typedef struct thread_args {
    char* file;
    size_t size;
    ssize_t cursor;
    size_t end;
} thread_args;

typedef struct name_temp {
    char* name;
    int temp;
} name_temp;

void spin_lock(pair* p) {
    while (atomic_exchange(&p->locked, true)) {
        // Spin until the lock is released
        while (atomic_load(&p->locked)) {
            // Do nothing, just spin
        }
    }
}

void spin_unlock(pair* p) { atomic_store(&p->locked, false); }

int worker(void* arg) {
    thread_args ta = *(thread_args*)arg;
    if (ta.cursor != 0) {
        while (ta.cursor > 0 && ta.file[ta.cursor] != '\n') {
            ta.cursor--;
        }
        ta.cursor++;
    }

    // Cursor is now pointing to the beginning of a line
    char name[100];
    char temp[6];
    while (ta.cursor < ta.end) {
        int name_pos = 0;
        int temp_pos = 0;
        while (ta.file[ta.cursor] != ';') {
            name[name_pos++] = ta.file[ta.cursor++];
        }
        name[name_pos] = '\0';
        ta.cursor++;
        while (ta.file[ta.cursor] != '\n') {
            temp[temp_pos++] = ta.file[ta.cursor++];
        }
        temp[temp_pos] = '\0';
        ta.cursor++;
        int temp_i = parse_int(temp);
        if (ta.cursor >= ta.end) {
            return 0;
        }
        pair* p = hashmap_find(&h, name);
        if (p) {
            spin_lock(p);
            p->value.count++;
            p->value.total += temp_i;
            if (temp_i > p->value.max) {
                p->value.max = temp_i;
            } else if (temp_i < p->value.min) {
                p->value.min = temp_i;
            }
            spin_unlock(p);
        } else {
            char* name_copy = strdup(name);
            hashmap_insert(&h, name_copy,
                           (Record){.name = name_copy,
                                    .total = temp_i,
                                    .count = 1,
                                    .min = temp_i,
                                    .max = temp_i});
        }
    }

    return 0;
}

int main() {
    size_t thread_count = 12;
    thrd_t threads[thread_count];

    h = hashmap_init(&fnv_string_hash, &str_equals, 0.5, 17729);

    char* file;
    int fd;
    struct stat sb;

    fd = open("./data/measurements.txt", O_RDONLY);

    fstat(fd, &sb);

    file = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    close(fd);

    ssize_t remaining_size = sb.st_size;
    size_t work_size = sb.st_size / thread_count;

    for (int i = 0; i < thread_count; i++) {
        thread_args* ta = malloc(sizeof(thread_args));
        *ta = (thread_args){.file = file,
                            .size = sb.st_size,
                            .cursor = i * work_size,
                            .end = (i * work_size) + work_size};
        thrd_create(&threads[i], worker, ta);
    }

    for (int i = 0; i < thread_count; i++) {
        thrd_join(threads[i], NULL);
    }

    qsort(h.array, h.capacity, sizeof(pair), compare_pairs);

    for (size_t i = 0; i < h.capacity && h.array[i].key; i++) {
        printf("%s=%.1f/%.1f/%.1f, ", (char*)h.array[i].key,
               (float)h.array[i].value.min / 10,
               (float)h.array[i].value.total / 10 /
                   (float)h.array[i].value.count,
               (float)h.array[i].value.max / 10);
        /* free(h.array[i].key); */
    }

    munmap(file, sb.st_size);
    hashmap_free(&h);
}
