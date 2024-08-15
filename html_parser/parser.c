#include "parser.h"
#include <stdbool.h>

bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

char parser_peek(parser* p) { return p->input[p->cursor]; }

char parser_get(parser* p) { return p->input[p->cursor++]; }

void parser_skip_whitespace(parser* p) {
    while (is_whitespace(parser_peek(p))) {
        parser_get(p);
    }
}

void parser_free(parser p) { free(p.input); }
