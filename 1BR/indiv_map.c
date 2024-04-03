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
    hashmap* internal_hm;
} thread_args;

typedef struct name_temp {
    char* name;
    int temp;
} name_temp;

int worker(void* arg) {
    thread_args ta = *(thread_args*)arg;
    *ta.internal_hm = hashmap_init(&fnv_string_hash, &str_equals, 0.5, 30011);
    if (ta.cursor != 0) {
        while (ta.file[ta.cursor - 1] != '\n') {
            ta.cursor--;
        }
    }

    // Cursor is now pointing to the beginning of a line
    char name[100];
    char temp[6];
    while (ta.cursor < ta.end) {
        int name_pos = 0;
        int temp_pos = 0;
        char current_char;
        while ((current_char = ta.file[ta.cursor]) != ';') {
            name[name_pos++] = current_char;
            ta.cursor++;
        }
        name[name_pos] = '\0';
        ta.cursor++;
        while ((current_char = ta.file[ta.cursor]) != '\n') {
            temp[temp_pos++] = current_char;
            ta.cursor++;
        }
        temp[temp_pos] = '\0';
        ta.cursor++;
        int temp_i = parse_int(temp);
        if (ta.cursor >= ta.end) {
            return 0;
        }
        pair* p = hashmap_find(ta.internal_hm, name);
        p->value.count++;
        p->value.total += temp_i;
        int max_condition =
            (temp_i > p->value.max); // 1 if temp_i > p->value.max, 0 otherwise
        int min_condition =
            (temp_i < p->value.min); // 1 if temp_i < p->value.min, 0 otherwise

        p->value.max =
            (max_condition * temp_i) + ((1 - max_condition) * p->value.max);
        p->value.min =
            (min_condition * temp_i) + ((1 - min_condition) * p->value.min);
    }

    return 0;
}

int main() {
    size_t thread_count = 12;
    thrd_t threads[thread_count];
    hashmap hashmaps[thread_count];

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
                            .end = (i * work_size) + work_size,
                            .internal_hm = &hashmaps[i]};
        thrd_create(&threads[i], worker, ta);
    }

    for (int i = 0; i < thread_count; i++) {
        thrd_join(threads[i], NULL);
    }

    int total_collisions = hashmaps[0].collisions;

    for (int i = 1; i < thread_count; i++) {
        for (int j = 0; j < hashmaps[i].capacity; j++) {
            if (hashmaps[i].array[j].key == 0) {
                continue;
            }
            pair* p = hashmap_find(&hashmaps[0], hashmaps[i].array[j].key);
            p->value.count += hashmaps[0].array[j].value.count;
            p->value.total += hashmaps[0].array[j].value.total;
            if (hashmaps[0].array[j].value.max > p->value.max) {
                p->value.max = hashmaps[0].array[j].value.max;
            }
            if (hashmaps[0].array[j].value.min < p->value.min) {
                p->value.min = hashmaps[0].array[j].value.min;
            }
        }
	
	total_collisions += hashmaps[i].collisions;
    }

    qsort(hashmaps[0].array, hashmaps[0].capacity, sizeof(pair), compare_pairs);

    for (size_t i = 0; i < hashmaps[0].capacity && hashmaps[0].array[i].key;
         i++) {
        printf("%s=%.1f/%.1f/%.1f, ", (char*)hashmaps[0].array[i].key,
               (float)hashmaps[0].array[i].value.min / 10,
               (float)hashmaps[0].array[i].value.total / 10 /
                   (float)hashmaps[0].array[i].value.count,
               (float)hashmaps[0].array[i].value.max / 10);
        /* free(h.array[i].key); */
    }

    printf("Average collisions: %f\n", (double)total_collisions / (double)thread_count);


    /* munmap(file, sb.st_size); */
    /* hashmap_free(&h); */
}
