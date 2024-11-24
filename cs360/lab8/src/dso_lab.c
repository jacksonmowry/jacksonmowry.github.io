// Jackson Mowry
// Sun Nov 24 09:48:17 2024
// A plugin loader lab
#include <assert.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    type vector_##type##_remove(vector_##type* v, size_t index);               \
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
    type vector_##type##_remove(vector_##type* v, size_t index) {              \
        type ret_val = v->array[index];                                        \
        memmove(v->array + index, v->array + index + 1,                        \
                sizeof(type) * (v->len - index - 1));                          \
        v->len--;                                                              \
        return ret_val;                                                        \
    }                                                                          \
    void vector_##type##_free(vector_##type v) { free(v.array); }

typedef struct Plugin {
    char name[65];         // Given by plugin designer
    int (*init)(void);     // Init function to call when plugin is loaded, 0 on
                           // success otherwise error code
    void (*fini)(void);    // Called when plugin is unloaded
    int (*cmd)(char* str); // true (1) mean success no need for others, 0
                           // unsuccessful so try others
} Plugin;

// DSO is a simple way to package both the DSO handle and the symbol `export`
typedef struct DSO {
    Plugin* p;
    void* handle;
} DSO;

// Initializing the vector for our DSO
VECTOR_PROTOTYPE(DSO)
VECTOR(DSO)

int main(void) {
    // Creating our vector
    vector_DSO v = vector_DSO_init(1);

    while (true) {
        printf("> ");
        char input_buffer[256] = {0};
        int elements_read;
        elements_read = scanf("%255s", input_buffer);
        if (elements_read != 1) {
            // We're likely EOF here, we break to cleanup
            break;
        }

        if (!strcmp("quit", input_buffer)) {
            // Once again break out to cleanup
            break;
        } else if (!strcmp("load", input_buffer)) {
            // Adding a plugin, so we have to scan up to `PATH_MAX` for a
            // potential plugin to load
            char token_to_load[PATH_MAX] = {0};
            elements_read = scanf("%4095s", token_to_load);

            if (elements_read != 1) {
                continue;
            }

            void* plugin = dlopen(token_to_load, RTLD_LAZY);
            if (!plugin) {
                // I guess we're not using dlerror? Marz just prints out this
                // generic message
                printf("%s: unable to load\n", token_to_load);
                continue;
            }

            Plugin* p = dlsym(plugin, "export");
            if (!p) {
                printf("%s\n", dlerror());
                continue;
            }
            if (p->init && p->init() != 0) {
                // Once again if it is not a valid plugin we can error out
                printf("%s: unable to load\n", token_to_load);
                dlclose(plugin);
                continue;
            }

            // Finally add this onto our vector
            vector_DSO_pb(&v, (DSO){.handle = plugin, .p = p});
        } else if (!strcmp("unload", input_buffer)) {
            // Once again we need to grab the name, this time only up to 64
            // characters
            char token_to_unload[65] = {0};
            elements_read = scanf("%64s", token_to_unload);

            if (elements_read != 1) {
                continue;
            }

            // Find it in the array to grab its index then remove it
            // I should probably have another way to remove items by some user
            // provided function by oh well
            size_t idx;
            for (idx = 0; idx < v.len; idx++) {
                if (!strcmp(v.array[idx].p->name, token_to_unload)) {
                    break;
                }
            }

            if (idx == v.len) {
                continue;
            }

            DSO d = vector_DSO_remove(&v, idx);

            d.p->fini();
            dlclose(d.handle);
        } else if (!strcmp("list", input_buffer) ||
                   !strcmp("plugins", input_buffer)) {
            // Print everything out
            for (size_t i = 0; i < v.len; i++) {
                printf("%s\n", v.array[i].p->name);
            }

            printf("%lu plugins loaded.\n", v.len);
        } else if (strlen(input_buffer)) {
            // The last possibility is to pass a command along to the loaded DSO
            int code = 0;
            for (size_t i = 0; code == 0 && i < v.len; i++) {
                code = v.array[i].p->cmd(input_buffer);
            }
        }
    }

    // Finally we cleanup
    for (size_t i = 0; i < v.len; i++) {
        v.array[i].p->fini();
        dlclose(v.array[i].handle);
    }

    vector_DSO_free(v);
}
