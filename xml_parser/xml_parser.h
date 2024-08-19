#ifndef FOO_H
#define FOO_H
#pragma once

#include <stdbool.h>
#include <stddef.h>
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
    void vector_##type##_free(vector_##type v);

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

kv_pair plist_get_pair(plist* p, size_t pos);
void plist_free(plist* p);
const char* plist_get_val(plist* p, char* key);

/*********************/
/* generic element_t */
/*********************/
typedef enum element_types { OPENING_TAG, CLOSING_TAG } element_type;

typedef enum text_type { PLAIN_TEXT, COMMENT } text_type;

typedef struct vector_element_t vector_element_t;

VECTOR_PROTOTYPE(char);

typedef struct element {
    element_type type;
    char* element_type;
    plist* properties;
    vector_element_t* children;
} element;

typedef struct text_content {
    text_type type;
    vector_char content;
} text_content;

typedef union element_u {
    element e;
    text_content tc;
} element_u;

typedef enum element_e { EMPTY, ELEMENT, TEXT_CONTENT, END_OF_FILE } element_e;

typedef struct element_t {
    element_e tag;
    element_u e;
} element_t;

void element_t_print(element_t e);
void element_t_free(element_t e);

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

element_t next_element(streaming_parser* p);
void print_document(streaming_parser* p);

#endif // FOO_H
