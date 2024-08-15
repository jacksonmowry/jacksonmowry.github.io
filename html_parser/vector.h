#ifndef VECTOR_H_
#define VECTOR_H_

#include "parser.h"
#include "plist.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define sizeof_thing sizeof(uint32_t)
#define GROWTH_FACTOR 2

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
    vector_##type vector_##type##_from(const type* input, size_t size) {       \
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

typedef enum type {
    NO_CONTENT,
    PLAIN_TEXT,
    OPENING_TAG,
    CLOSING_TAG,
    COMMENT
} type;

typedef struct element_types {
    size_t size;
    size_t cap;
    char* types[];
} types;

typedef uint16_t* thing;

typedef struct tag_size {
    uint16_t type;
    int16_t element_type;
    uint16_t size;
    uint16_t cap;
} tag_size;

typedef struct vector_element_t vector_element_t;

typedef struct element {
    tag_size tag_size;
    plist* properties;
    vector_element_t* children;
} element;

VECTOR(char);

typedef struct text_content {
    tag_size tag_size;
    vector_char content;
} text_content;

typedef union element_u {
    tag_size ts;
    element e;
    text_content tc;
} element_u;

typedef enum element_e { TS, E, TC } element_e;

typedef struct element_t {
    element_e tag;
    element_u e;
} element_t;

VECTOR(element_t);

type get_type(element_t);
void set_type(element_t*, type);
uint16_t get_size(element_t);
void set_size(element_t*, uint16_t);
uint16_t get_cap(element_t);
void set_cap(element_t*, uint16_t);

const char* get_content(text_content*);
void set_content(text_content*, const char*, uint16_t);

const char* get_element_type(element*);

int16_t add_element_type(const char* type, size_t size);

element* parse_element(parser* p);
text_content* parse_text(parser* p);

element* parse_tag(parser* p);

bool closing_tag_p(const char*, size_t, parser*);

bool comment_tag(parser*);
text_content* parse_comment(parser*);
text_content* new_comment(const char*, size_t);

typedef enum field_type { i32, f32, string, substruct, timestamp } field_type;

typedef struct field {
    field_type type;
    char* substruct_type;
    char* name;
    char* default_value;
    bool required;
} field;

VECTOR(field);

typedef struct message {
    vector_field fields;
    char* name;
} message;

VECTOR(message);

typedef struct schema {
    vector_message messages;
    char* version;
} schema;

VECTOR(tag_size);

/* typedef struct tag_size_vector { */
/*     size_t size; */
/*     size_t cap; */
/*     tag_size** elements; */
/* } tag_size_vector; */

/* tag_size_vector tag_size_vector_init(); */
/* void tag_size_vector_free(tag_size_vector*); */
/* void tag_size_vector_pb(tag_size_vector*, tag_size*); */
#endif // VECTOR_H_
