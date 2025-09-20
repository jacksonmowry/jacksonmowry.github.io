#include <stdio.h>

void match(const char c);
void cond(void);

int lookahead;

void expr(void) {
    if (lookahead == EOF && lookahead != 'i') {
        // Return epsilon case
        return;
    }

    match('i');
    cond();
    match('t');
    if (lookahead != 'e') {
        // We have an expr
        // I'm assuming here we only want to print true if the expr does not
        // evaluate to epsilon? Could easily move that outside the if statement
        // if that assumption is false
        expr();
        puts("true");
    }

    match('e');
    if (lookahead != EOF) {
        // We have an expr
        // I'm assuming here we only want to print false if the expr does not
        // evaluate to epsilon? Could easily move that outside the if statement
        // if that assumption is false
        expr();
        puts("false");
    }
}
