#include "hashmap/hashmap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main() {
    hashmap h = hashmap_init(&fnv_string_hash, &str_equals, 0.5, 17720);

    FILE* fin;
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;

    fin = fopen("./data/measurements.txt", "r");

    while ((nread = getline(&line, &len, fin)) != -1) {
        char* copy = line;
        strtok(copy, ";");
        char* temp_s = strtok(NULL, "\n");
        float temp_f = strtof(temp_s, NULL);
        int temp_i = temp_f * 10;
        pair* p = hashmap_find(&h, copy);
        if (p) {
            p->value.count++;
            p->value.total += temp_i;
            if (temp_i > p->value.max) {
                p->value.max = temp_i;
            } else if (temp_i < p->value.min) {
                p->value.min = temp_i;
            }
        } else {
            hashmap_insert(&h, strdup(copy),
                           (Record){.name = copy,
                                    .total = temp_i,
                                    .count = 1,
                                    .max = temp_i,
                                    .min = temp_i});
        }
    }

    qsort(h.array, h.capacity, sizeof(pair), compare_pairs);

    for (size_t i = 0; i < h.capacity && h.array[i].key; i++) {
        printf("%s=%.1f/%.1f/%.1f, ", (char*)h.array[i].key,
               (float)h.array[i].value.min / 10,
               (float)h.array[i].value.total / 10 /
                   (float)h.array[i].value.count,
               (float)h.array[i].value.max / 10);
        free(h.array[i].key);
    }

    free(line);
    fclose(fin);
    hashmap_free(&h);
}
