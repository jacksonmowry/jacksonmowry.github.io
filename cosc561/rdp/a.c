#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void expr();
void term();
void match(int t);
void error();

int lookahead;

int main() {
    lookahead = getchar();

    expr();
    putchar('\n');
}

void expr() {
    term();
    while (true) {
        if (lookahead == '+') {
            match('+');
            term();
            putchar('+');
        } else if (lookahead == '-') {
            match('-');
            term();
            putchar('-');
        } else {
            // Empty string
            break;
        }
    }
}

void term() {
    if (isdigit(lookahead)) {
        putchar(lookahead);
        match(lookahead);
    } else {
        error();
    }
}

void match(int t) {
    if (lookahead == t) {
        lookahead = getchar();
    } else {
        error();
    }
}

void error() {
    fprintf(stderr, "syntax error\n");
    exit(1);
}
