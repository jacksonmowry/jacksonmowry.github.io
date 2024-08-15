#ifndef PARSER_H_
#define PARSER_H_

#include <stdbool.h>
#include <stdlib.h>

typedef struct parser {
    char* input;
    char next;
    size_t cursor;
    size_t len;
} parser;

bool is_whitespace(char);
char parser_peek(parser*);
char parser_get(parser*);
void parser_skip_whitespace(parser*);
void parser_free(parser);

#endif // PARSER_H_
