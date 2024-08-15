#include "plist.h"
#include <string.h>

plist* plist_init() {
    plist* p = calloc(1, sizeof(plist));
    p->size = 0;
    p->cap = 1;
    p->list = calloc(2, sizeof(char*));

    return p;
}

// Also frees the plist
void plist_free(plist* p) {
    for (int i = 0; i < p->size; i++) {
        kv_pair kv = plist_get_pair(p, i);
        free(kv.key);
        free(kv.val);
    }
    free(p->list);
    free(p);
}

void plist_add(plist* p, char* key, char* value) {
    if (p->size + 1 >= p->cap) {
        p->cap *= 2;
        p->list = realloc(p->list, p->cap * sizeof(char*));
    }

    p->list[p->size * 2] = key;
    p->list[p->size * 2 + 1] = value;
    p->size++;
}

const char* plist_get_val(plist* p, char* key) {
    if (p->size <= 0) {
        return NULL;
    }

    for (size_t i = 0; i < p->size; i++) {
        if (!strcmp(p->list[i * 2], key)) {
            return p->list[i * 2 + 1];
        }
    }

    return NULL;
}

kv_pair plist_get_pair(plist* p, size_t pos) {
    if (pos >= p->size) {
        return (kv_pair){.key = NULL, .val = NULL};
    }

    return (kv_pair){.key = p->list[pos * 2], .val = p->list[pos * 2 + 1]};
}
