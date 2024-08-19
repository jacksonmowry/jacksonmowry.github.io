#include "xml_parser.h"
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef char* string;
VECTOR_PROTOTYPE(string);
VECTOR_PROTOTYPE(element_t);

/***********************/
/* generic vector impl */
/***********************/
#define VECTOR(type)                                                           \
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

VECTOR(char);
VECTOR(string);
/*********/
/* plist */
/*********/
plist* plist_init() {
    plist* p = calloc(1, sizeof(plist));
    if (!p) {
        fprintf(stderr, "%s:%d: Failed to allocate memory for plist", __FILE__,
                __LINE__);
        exit(1);
    }
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
    for (size_t i = 0; i < p->size; i++) {
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

vector_string tag_type_cache = {};

char* get_element_tag(const char* const tag, size_t size) {
    // On first we need to init the global variable
    if (tag_type_cache.cap == 0) {
        tag_type_cache = vector_string_init(10);
    }

    // Try to find the tag, otherwise make a copy and return that pointer
    for (size_t i = 0; i < tag_type_cache.len; i++) {
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
VECTOR(element_t);

/***********************/
/* element_t functions */
/***********************/
element_t new_empty() { return (element_t){.tag = EMPTY}; }
element_t new_eof() { return (element_t){.tag = END_OF_FILE}; }

void element_t_print(element_t e) {
    switch (e.tag) {
    case ELEMENT: {
        printf("<");

        switch (e.e.e.type) {
        case OPENING_TAG: {
            printf("%s", e.e.e.element_type);

            for (size_t i = 0; i < e.e.e.properties->size; i++) {
                kv_pair kv = plist_get_pair(e.e.e.properties, i);
                printf(" %s=\"%s\"", kv.key, kv.val);
            }
        } break;
        case CLOSING_TAG: {
            printf("/%s", e.e.e.element_type);
        } break;
        }

        printf(">");
    } break;
    case TEXT_CONTENT: {
        switch (e.e.tc.type) {
        case PLAIN_TEXT: {
            printf("%.*s", (int)e.e.tc.content.len, e.e.tc.content.array);
        } break;
        case COMMENT: {
            printf("<!-- %.*s -->", (int)e.e.tc.content.len,
                   e.e.tc.content.array);
        } break;
        }
    }
    default:
        break;
    }
}

void element_t_free(element_t e) {
    switch (e.tag) {
    case ELEMENT: {
        switch (e.e.e.type) {
        case OPENING_TAG: {
            plist_free(e.e.e.properties);
            vector_element_t_free(*e.e.e.children);
            free(e.e.e.children);
        } break;
        case CLOSING_TAG: {
        } break;
        }
    } break;
    case TEXT_CONTENT: {
        vector_char_free(e.e.tc.content);
    }
    default:
        break;
    }
}

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

    if (!e.e.e.children) {
        fprintf(
            stderr,
            "%s:%d: Failed to allocate memory for .children in opening tag\n",
            __FILE__, __LINE__);
        exit(1);
    }

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

bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

char parser_get(streaming_parser* p) {
    if (p->position == p->length - 1) {
        assert("Reached the end of input" == false);
    }

    char ret = p->current;
    p->current = p->input[++p->position];
    return ret;
}

bool streaming_parser_skip_whitespace(streaming_parser* p) {
    while (is_whitespace(p->current) && p->position < p->length - 1) {
        parser_get(p);
    }

    if (p->position >= p->length - 1) {
        return true;
    }

    return false;
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
    //        ^          ^
    char* token = strndup(&p->input[start], p->position - start);
    if (!token) {
        fprintf(stderr, "%s:%s:%d: Failed to call 'strndup' to copy token",
                __FILE__, __FUNCTION__, __LINE__);
        exit(1);
    }

    return token;
}

// value=42>This is some text<span class=""
//          ^
element_t parse_text(streaming_parser* p, const char* delimiter,
                     size_t delimiter_length) {
    if (streaming_parser_skip_whitespace(p)) {
        return new_eof();
    }

    size_t start = p->position;

    while (strncmp(&p->input[p->position], delimiter, delimiter_length)) {
        parser_get(p);
    }

    if (p->position - start == 0) {
        return new_empty();
    }

    return new_plain_text(&p->input[start], p->position - start);
}

// <!-- This is a comment -->
//      ^
element_t parse_comment(streaming_parser* p) {
    element_t comment = parse_text(p, "-->", 3);
    parser_get(p);
    parser_get(p);
    parser_get(p);

    return comment;
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
        assert("Prop values must be strings!" == false);
    }

    // class="myClass" or class = myClass>
    //                ^                  ^
    return (kv_pair){.key = key, .val = value};
}

// </div>
//  ^
element_t parse_closing_tag(streaming_parser* p) {
    assert(p->current == '/');
    parser_get(p);

    char* tag_name = parse_token(p);
    parser_get(p);

    element_t closing_tag = new_closing_tag(tag_name, strlen(tag_name));
    free(tag_name);

    return closing_tag;
}

// <div class="myClass" id="opening">
// ^ cursor starts here              ^ we return the cursor here
element_t parse_opening_tag(streaming_parser* p) {
    parser_get(p);

    switch (p->current) {
    case '!':
        return parse_comment(p);
    case '/':
        return parse_closing_tag(p);
    default:
        break;
    }

    // Record start of tag name
    size_t start = p->position;
    while (!is_whitespace(p->current) && p->current != '>') {
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
        streaming_parser_skip_whitespace(p);
    }

    assert(p->current == '>');
    parser_get(p);

    return element;
}

element_t next_element(streaming_parser* p) {
    if (streaming_parser_skip_whitespace(p)) {
        return new_eof();
    }

    if (p->current == '<') {
        return parse_opening_tag(p);
    } else {
        element_t thing = parse_text(p, "<", 1);

        if (thing.tag == EMPTY) {
            element_t_free(thing);
            return next_element(p);
        }

        return thing;
    }
}

void print_indent(size_t depth) {
    for (size_t i = 0; i < depth; i++) {
        printf(" ");
    }
}

void print_document(streaming_parser* p) {
    size_t depth = 0;
    while (p->position < p->length) {
        element_t e = next_element(p);
        if (e.tag == END_OF_FILE) {
            break;
        }

        if (e.tag == ELEMENT && e.e.e.type == CLOSING_TAG) {
            depth -= 4;
        }

        print_indent(depth);
        element_t_print(e);

        if (e.tag == ELEMENT && e.e.e.type == OPENING_TAG) {
            depth += 4;
        }
        printf("\n");

        element_t_free(e);
    }
}
