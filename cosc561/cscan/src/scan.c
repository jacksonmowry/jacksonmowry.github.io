#include "scan.h"
#include <stdio.h>

int lookahead;
char* buf;
size_t buf_len;

void error() {
    printf("Something bad happened\n");
    exit(1);
}

void match(char expected) {
    if (lookahead == expected) {
        lookahead = getchar();
    } else {
        error();
    }
}

void init_scanner() {
    lookahead = getchar();

    buf = malloc(1 << 24);
    buf_len = 0;
}

Token get_token() {
    return (Token){.type = RBRACE, .lexeme = "}"};
    switch (lookahead) {
    case '/': {
        match('/');
    } break;
    }
}
