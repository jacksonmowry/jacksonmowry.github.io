// Jackson Mowry
// Thu Oct 24 11:24:42 2024
// A program to hash a directory

#include "tpool.h"

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

/*********************************/
/* generic vector implementation */
/*********************************/
#define VECTOR_PROTOTYPE(type)                                                 \
    typedef struct vector_##type {                                             \
        type* array;                                                           \
        size_t len;                                                            \
        size_t cap;                                                            \
    } vector_##type;                                                           \
    void vector_##type##_pb(vector_##type* v, type item);                      \
    type vector_##type##_get(vector_##type* v, size_t index);                  \
    vector_##type vector_##type##_init(size_t cap);                            \
    vector_##type vector_##type##_from(const type* const input, size_t size);  \
    bool vector_##type##_eq(vector_##type a, vector_##type b,                  \
                            bool (*equality_func)(void*, void*));              \
    bool vector_##type##_cmp(vector_##type a, vector_##type b);                \
    void vector_##type##_free(vector_##type v);

#define VECTOR(type)                                                           \
    void vector_##type##_pb(vector_##type* v, type item) {                     \
        if (!v) {                                                              \
            fprintf(stderr, "Attempting to call pb on a NULL vector\n");       \
            exit(1);                                                           \
        }                                                                      \
        if (!v->array) {                                                       \
            fprintf(stderr,                                                    \
                    "Attempting to call pb on a vector with NULL array\n");    \
            exit(1);                                                           \
        }                                                                      \
        if (v->len >= v->cap) {                                                \
            v->cap *= 2;                                                       \
            v->array = (type*)realloc(v->array, v->cap * sizeof(type));        \
        }                                                                      \
        v->array[v->len++] = item;                                             \
    }                                                                          \
    type vector_##type##_get(vector_##type* v, size_t index) {                 \
        if (index >= v->len) {                                                 \
            fprintf(stderr,                                                    \
                    "Attempt to access memory oob for v[%zu], index %zu\n",    \
                    v->len, index);                                            \
            exit(1);                                                           \
        }                                                                      \
        return v->array[index];                                                \
    }                                                                          \
    vector_##type vector_##type##_init(size_t cap) {                           \
        return (vector_##type){.array = (type*)calloc(cap, sizeof(type)),      \
                               .cap = cap};                                    \
    }                                                                          \
    vector_##type vector_##type##_from(const type* const input, size_t size) { \
        vector_##type blank = vector_##type##_init(size);                      \
        if (!blank.array) {                                                    \
            fprintf(stderr, "Failed to allocate vector_##type##_init\n");      \
            exit(1);                                                           \
        }                                                                      \
        blank.len = size;                                                      \
        memcpy(blank.array, input, sizeof(type) * size);                       \
        return blank;                                                          \
    }                                                                          \
    bool vector_##type##_eq(vector_##type a, vector_##type b,                  \
                            bool (*equality_func)(void*, void*)) {             \
        if (a.len != b.len) {                                                  \
            return false;                                                      \
        }                                                                      \
        for (size_t i = 0; i < a.len; i++) {                                   \
            if (!equality_func(&a.array[i], &b.array[i])) {                    \
                return false;                                                  \
            }                                                                  \
        }                                                                      \
        return true;                                                           \
    }                                                                          \
    bool vector_##type##_cmp(vector_##type a, vector_##type b) {               \
        if (a.len != b.len) {                                                  \
            return false;                                                      \
        }                                                                      \
        return !memcmp(a.array, b.array, sizeof(type) * a.len);                \
    }                                                                          \
    void vector_##type##_free(vector_##type v) { free(v.array); }

typedef struct {
    union {
        uint32_t small_hash;
        uint64_t big_hash;
    } hash;

    char* file_name;
} hash_result;

typedef struct {
    char* file_name;
    enum { SMALL, LARGE } hash_type;
    size_t result_index;
} Work;

VECTOR_PROTOTYPE(hash_result)
VECTOR(hash_result)

typedef struct TPool {
    pthread_t* tids;
    size_t num_tids;

    bool die;

    Work** buf;
    size_t size;
    size_t capacity;
    size_t at;

    vector_hash_result vhr;

    int outstanding_work;

    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    pthread_mutex_t mutex;
} TPool;

// `hash32` creates a 32-bit digest of the file provded by `fl`
uint32_t hash32(FILE* fl) {
    uint32_t digest = 2166136261;

    uint8_t byte;
    while (fread(&byte, 1, 1, fl)) {
        digest = (digest ^ byte) * 16777619;
    }

    return digest;
}

// `hash64` creates a 64-bit digest of the file provded by `fl`
uint64_t hash64(FILE* fl) {
    uint64_t digest = 14695981039346656037UL;

    uint8_t byte;
    while (fread(&byte, 1, 1, fl)) {
        digest = (digest ^ byte) * 1099511628211;
    }

    return digest;
}

// `worker` is a pthread worker function, it is provided a TPool* via `arg`
// which contains all of the shared memory needed
void* worker(void* arg) {
    TPool* tp = (TPool*)arg;

    while (true) {
        // Only lock to mutex to grab work, then unlock once the buffer has
        // moved
        pthread_mutex_lock(&tp->mutex);

        while (tp->size == 0 && !tp->die) {
            pthread_cond_wait(&tp->not_empty, &tp->mutex);
        }

        if (tp->die) {
            pthread_mutex_unlock(&tp->mutex);
            return NULL;
        }

        Work* w = tp->buf[tp->at];
        tp->at = (tp->at + 1) % tp->capacity;
        tp->size -= 1;

        pthread_cond_signal(&tp->not_full);
        pthread_mutex_unlock(&tp->mutex);

        // Perform hashing outside of locked sections
        FILE* fp = fopen(w->file_name, "r");
        if (!fp) {
            free(w);
            continue;
        }
        uint32_t digest_small;
        uint64_t digest_big;
        if (w->hash_type == SMALL) {
            digest_small = hash32(fp);
        } else {
            digest_big = hash64(fp);
        }

        fclose(fp);

        // Relock the shared structure to write out result
        pthread_mutex_lock(&tp->mutex);
        // Insert in vec
        if (w->hash_type == SMALL) {
            tp->vhr.array[w->result_index].hash.small_hash = digest_small;
        } else {
            tp->vhr.array[w->result_index].hash.big_hash = digest_big;
        }

        // Decrement outstanding work counter by one
        tp->outstanding_work -= 1;
        pthread_mutex_unlock(&tp->mutex);

        free(w);
    }

    // Just in case the look somehow ends
    pthread_mutex_unlock(&tp->mutex);
    return NULL;
}

// `thread_pool_init` sets up an opaque data structure to hold all threads, and
// their associated work queue.
// Threads are created within this function
void* thread_pool_init(int num_threads) {
    if (num_threads < 1 || num_threads > 32) {
        return NULL;
    }

    TPool* tp = calloc(1, sizeof(TPool));
    if (!tp) {
        return NULL;
    }

    tp->tids = calloc(num_threads, sizeof(pthread_t));
    if (!tp->tids) {
        free(tp);
        return NULL;
    }
    tp->num_tids = num_threads;

    tp->size = 0;
    tp->capacity = num_threads;
    tp->at = 0;
    tp->buf = calloc(num_threads, sizeof(Work*));
    tp->die = false;

    tp->vhr = vector_hash_result_init(num_threads);

    pthread_cond_init(&tp->not_empty, NULL);
    pthread_cond_init(&tp->not_full, NULL);
    pthread_mutex_init(&tp->mutex, NULL);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(tp->tids + i, NULL, worker, tp);
    }

    return tp;
}

// `thread_pool_hash` takes in an opaque structure (TPool*), a `directory` to
// hash, and the size of each hash as `hash_size`
// Results are printed to `stdout` in directory order, and threads are kept
// alive once they complete work
bool thread_pool_hash(void* handle, const char* directory, int hash_size) {
    TPool* tp = (TPool*)handle;
    if (!tp) {
        // We cannot continue, return false
        return false;
    }

    tp->vhr.len = 0;
    tp->outstanding_work = 0;

    // If the handle is NULL, if the directory is invalid, or the hash_size is
    // not [32, 64], then this function will return false.
    if (!handle || !directory || !strlen(directory) ||
        (hash_size != 32 && hash_size != 64)) {
        // We cannot continue, return false
        return false;
    }

    DIR* d;
    if ((d = opendir(directory)) == NULL) {
        // We were unable to open the provided directory
        // We cannot continue, return false
        return false;
    }

    struct dirent* de;

    while ((de = readdir(d)) != NULL) {
        char* full_path = calloc(strlen(directory) + strlen(de->d_name) + 2, 1);
        if (!full_path) {
            closedir(d);
            // We cannot continue, return false
            return false;
        }

        // Construct the full path to make our lives easier later on
        strcat(full_path, directory);
        strcat(full_path, "/");
        strcat(full_path, de->d_name);

        struct stat st;
        stat(full_path, &st);

        // Skip over all non-regular files
        if (!S_ISREG(st.st_mode)) {
            free(full_path);
            continue;
        }

        // Enter our critial section to wait for our queue to have space
        pthread_mutex_lock(&tp->mutex);

        while (tp->size == tp->capacity) {
            pthread_cond_wait(&tp->not_full, &tp->mutex);
        }

        // Give this piece of work a "landing zone" to place its result
        vector_hash_result_pb(&tp->vhr, (hash_result){.hash.big_hash = 0,
                                                      .file_name = full_path});

        Work* w = malloc(sizeof(Work));
        if (!w) {
            free(full_path);
            closedir(d);
            // We cannot continue, return false
            return false;
        }
        w->file_name = full_path;
        w->hash_type = hash_size == 32 ? SMALL : LARGE;
        // We use indexes and not pointers here to avoid referencing memory that
        // has been reallocated by vector push_back
        w->result_index = tp->vhr.len - 1;

        tp->buf[(tp->size + tp->at) % tp->capacity] = w;
        tp->size += 1;

        tp->outstanding_work += 1;

        pthread_cond_signal(&tp->not_empty);
        pthread_mutex_unlock(&tp->mutex);
    }

    // close the directory to free up dynamically allocated resources as soon as
    // we're done
    closedir(d);

    while (true) {
        // Wait until all threads are finished working
        // Lock here to avoid contention between this producer and any consumers
        pthread_mutex_lock(&tp->mutex);
        bool cond = tp->outstanding_work > 0;
        pthread_mutex_unlock(&tp->mutex);
        if (cond) {
            // Sleep to not explode our running time
            usleep(100);
        } else {
            // All threads are done
            break;
        }
    }

    // Print results
    for (size_t i = 0; i < tp->vhr.len; i++) {
        if (hash_size == 32) {
            printf("%08x: %s\n", tp->vhr.array[i].hash.small_hash,
                   tp->vhr.array[i].file_name);
        } else {
            printf("%016lx: %s\n", tp->vhr.array[i].hash.big_hash,
                   tp->vhr.array[i].file_name);
        }

        free(tp->vhr.array[i].file_name);
    }

    // We're finished
    return true;
}

// `thread_pool_shutdown` frees up all dynamically allocated resources, and
// destroys all pthreads and their associated locking primitives
void thread_pool_shutdown(void* handle) {
    TPool* tp = (TPool*)handle;

    if (!tp) {
        // early exit
        return;
    }

    pthread_mutex_lock(&tp->mutex);
    tp->die = true;
    pthread_mutex_unlock(&tp->mutex);
    pthread_cond_broadcast(&tp->not_empty);

    for (size_t i = 0; i < tp->num_tids; i++) {
        pthread_join(tp->tids[i], NULL);
    }

    pthread_cond_destroy(&tp->not_full);
    pthread_cond_destroy(&tp->not_empty);
    pthread_mutex_destroy(&tp->mutex);

    free(tp->buf);
    vector_hash_result_free(tp->vhr);

    return;
}
