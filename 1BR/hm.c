#include "hashmap/hashmap.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
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

int main() {
    hashmap h = hashmap_init(&fnv_string_hash, &str_equals, 0.5, 17729);

    char* file;
    int fd;
    struct stat sb;

    fd = open("./data/measurements.txt", O_RDONLY);

    fstat(fd, &sb);

    file = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    size_t cursor = 0;
    char name[100];
    char temp[6];
    while (cursor < sb.st_size) {
        int name_pos = 0;
        int temp_pos = 0;
        while (file[cursor] != ';') {
            name[name_pos++] = file[cursor++];
        }
        name[name_pos] = '\0';
        cursor++;
        while (file[cursor] != '\n') {
            temp[temp_pos++] = file[cursor++];
        }
        temp[temp_pos] = '\0';
        cursor++;
        int temp_i = parse_int(temp);

        pair* p = hashmap_find(&h, strdup(name));
        p->value.count++;
        p->value.total += temp_i;
        if (temp_i > p->value.max) {
            p->value.max = temp_i;
        } else if (temp_i < p->value.min) {
            p->value.min = temp_i;
        }
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
