#ifndef PLIST_H_
#define PLIST_H_

#include <stdlib.h>

typedef struct kv_pair {
    char* key;
    char* val;
} kv_pair;

typedef struct plist {
    size_t size;
    size_t cap;
    char** list;
} plist;

plist* plist_init();
void plist_free(plist*);
void plist_add(plist* p, char* key, char* value);
const char* plist_get(plist* p, char* key);
kv_pair plist_get_pair(plist* p, size_t pos);

#endif // PLIST_H_
