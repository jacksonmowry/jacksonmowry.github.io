#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*********************************/
/* generic vector implementation */
/*********************************/
#define VECTOR(type)                                                           \
    typedef struct vector_##type {                                             \
        type* array;                                                           \
        size_t len;                                                            \
        size_t cap;                                                            \
    } vector_##type;                                                           \
    void vector_##type##_pb(vector_##type* v, type item) {                     \
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

/*********/
/* plist */
/*********/
typedef struct kv_pair {
    char* key;
    char* val;
} kv_pair;

typedef struct plist {
    size_t size;
    size_t cap;
    char** list;
} plist;

plist* plist_init() {
    plist* p = calloc(1, sizeof(plist));
    p->size = 0;
    p->cap = 1;
    p->list = calloc(2, sizeof(char*));

    return p;
}

kv_pair plist_get_pair(plist* p, size_t pos) {
    if (pos >= p->size) {
        return (kv_pair){.key = NULL, .val = NULL};
    }

    return (kv_pair){.key = p->list[pos * 2], .val = p->list[pos * 2 + 1]};
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

/******************/
/* tag type cache */
/******************/
typedef char* string;
VECTOR(string);

vector_string tag_type_cache = {};

const char* const get_element_tag(const char* const tag, size_t size) {
    // On first we need to init the global variable
    if (tag_type_cache.cap == 0) {
        tag_type_cache = vector_string_init(10);
    }

    // Try to find the tag, otherwise make a copy and return that pointer
    for (int i = 0; i < tag_type_cache.len; i++) {
        if (!strcmp(tag_type_cache.array[i], tag)) {
            return tag_type_cache.array[i];
        }
    }

    vector_string_pb(&tag_type_cache, strndup(tag, size));
    return tag_type_cache.array[tag_type_cache.len - 1];
}

/*********************/
/* generic element_t */
/*********************/
typedef enum type { PLAIN_TEXT, OPENING_TAG, CLOSING_TAG, COMMENT } type;

typedef struct vector_element_t vector_element_t;
VECTOR(char);

typedef struct element {
    type type;
    char* element_type;
    plist* properties;
    vector_element_t* children;
} element;

typedef struct text_content {
    type type;
    vector_char content;
} text_content;

typedef union element_u {
    element e;
    text_content tc;
} element_u;

typedef enum element_e { EMPTY, ELEMENT, TEXT_CONTENT } element_e;

typedef struct element_t {
    element_e tag;
    element_u e;
} element_t;

VECTOR(element_t);

/***********************/
/* element_t functions */
/***********************/
element_t new_empty() { return (element_t){.tag = EMPTY}; }

/**************************/
/* text_content functions */
/**************************/
element_t new_plain_text(const char* const text, size_t size) {
    return (element_t){
        .tag = TEXT_CONTENT,
        .e.tc = (text_content){.type = PLAIN_TEXT,
                               .content = vector_char_from(text, size)}};
}

element_t new_comment(const char* const text, size_t size) {
    return (element_t){
        .tag = TEXT_CONTENT,
        .e.tc = (text_content){.type = COMMENT,
                               .content = vector_char_from(text, size)}};
}

/*********************/
/* element functions */
/*********************/
element_t new_opening_tag(const char* const tag, size_t length) {
    element_t e = {.tag = ELEMENT,
                   .e.e = {.element_type = get_element_tag(tag, length),
                           .type = OPENING_TAG,
                           .children = calloc(1, sizeof(vector_element_t)),
                           .properties = plist_init()}};
    *e.e.e.children = vector_element_t_init(1);
    return e;
}

element_t new_closing_tag(const char* const tag, size_t length) {
    return (element_t){.tag = ELEMENT,
                       .e.e = {.element_type = get_element_tag(tag, length),
                               .type = CLOSING_TAG}};
}

/***********************/
/* streaming_parser */
/***********************/

// either mmap a buffer for `input`, or directly pass a buffer
typedef struct streaming_parser {
    char* input;
    size_t position;
    size_t length;
    char current;
} streaming_parser;

bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

char parser_get(streaming_parser* p) {
    if (p->position == p->length - 1) {
        assert("Reached the end of input" == false);
    }

    char ret = p->current;
    p->current = p->input[++p->position];
    return ret;
}

void streaming_parser_skip_whitespace(streaming_parser* p) {
    while (is_whitespace(p->current)) {
        parser_get(p);
    }
}

// "myClass"
// ^
char* parse_string(streaming_parser* p) {
    assert(p->current == '"');
    parser_get(p);

    // "myClass"
    //  ^
    size_t start = p->position;
    while (p->current != '"') {
        parser_get(p);
    }
    assert(p->current == '"');

    char* string = strndup(&p->input[start], p->position - start);
    parser_get(p);

    return string;
}

// myClass
// ^
char* parse_token(streaming_parser* p) {
    size_t start = p->position;

    while (!is_whitespace(p->current) && p->current != '>') {
        parser_get(p);
    }

    // myClass or myClass>
    // ^          ^
    return strndup(&p->input[start], p->position - start);
}

// <div class="myClass" id="opening">
//      ^      or       ^
kv_pair parse_property(streaming_parser* p) {
    size_t start = p->position;
    char* key;
    char* value;

    // class="myClass" or class = "myClass"
    // ^                  ^
    while (!is_whitespace(p->current) && p->current != '=') {
        parser_get(p);
    }
    // class="myClass" or class = "myClass"
    //      ^                  ^
    key = strndup(&p->input[start], p->position - start);

    streaming_parser_skip_whitespace(p);
    // class="myClass" or class = "myClass"
    //      ^                   ^

    parser_get(p);
    // class="myClass" or class = "myClass"
    //       ^                   ^

    streaming_parser_skip_whitespace(p);
    // class="myClass" or class = myClass>
    //       ^                    ^

    if (p->current == '"') {
        // class="myClass"
        //       ^
        value = parse_string(p);
        // class="myClass"
        //                ^
    } else {
        // class = myClass>
        //         ^
        value = parse_token(p);
    }

    // class="myClass" or class = myClass>
    //                ^                  ^
    return (kv_pair){.key = key, .val = value};
}

// <div class="myClass" id="opening">
// ^ cursor starts here              ^ we return the cursor here
element_t parse_opening_tag(streaming_parser* p) {
    parser_get(p);

    // Record start of tag name
    size_t start = p->position;
    while (!is_whitespace(p->current)) {
        parser_get(p);
    }

    // <div class="myClass" id="opening">
    //     ^
    element_t element = new_opening_tag(&p->input[start], p->position - start);
    streaming_parser_skip_whitespace(p);

    // <div class="myClass" id="opening">
    //      ^
    // Loop through all props until we find `>`
    while (p->current != '>') {
        kv_pair kv = parse_property(p);
        plist_add(element.e.e.properties, kv.key, kv.val);
    }
}
