#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// (+ 1 2)
// (if 1 2 3)
// (and t f t)
// (or t f f)
// (lambda (x y z) (+ x y z))
// (let ((x 1) (y 2) (z 3)) (+ x y z))

typedef struct Parser {
    char *text;
    size_t pos;
} Parser;

char parser_peek(const Parser *p) { return p->text[p->pos]; }
char parser_get(Parser *p) { return p->text[p->pos++]; }
void parser_skip_whitespace(Parser *p) {
    while (p->text[p->pos] == ' ') {
        p->pos++;
    }
}

typedef struct Value {
    enum { NUMBER, STRING, BOOLEAN, PROCEDURE, SYMBOL } tag;
    union {
        double number;
        char *string;
        bool boolean;
    } val;
} Value;

Value t = (Value){.tag = BOOLEAN, .val.boolean = true};
Value f = (Value){.tag = BOOLEAN, .val.boolean = false};

Value parse(Parser *input);

Value handle_if(Parser *input) { return (Value){}; }
Value handle_and(Parser *input) { return (Value){}; }
Value handle_or(Parser *input) { return (Value){}; }
Value handle_lambda(Parser *input) { return (Value){}; }
Value handle_let(Parser *input) { return (Value){}; }

typedef enum BinaryOps {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
} BinaryOps;

Value handle_arithmetic(Parser *input, BinaryOps op) {
    parser_skip_whitespace(input);

    Value first = parse(input);
    assert(first.tag == NUMBER);

    while (parser_peek(input) != ')') {
        Value next = parse(input);
        assert(first.tag == NUMBER);

        switch (op) {
        case ADD:
            first.val.number += next.val.number;
            break;
        case SUB:
            first.val.number -= next.val.number;
            break;
        case MUL:
            first.val.number *= next.val.number;
            break;
        case DIV:
            first.val.number /= next.val.number;
            break;
        case MOD:
            first.val.number = fmod(first.val.number, next.val.number);
            break;
        }
    }

    return first;
}

Value handle_add(Parser *input) { return handle_arithmetic(input, ADD); }
Value handle_sub(Parser *input) { return handle_arithmetic(input, SUB); }
Value handle_mul(Parser *input) { return handle_arithmetic(input, MUL); }
Value handle_div(Parser *input) { return handle_arithmetic(input, DIV); }
Value handle_mod(Parser *input) { return handle_arithmetic(input, MOD); }

Value parse(Parser *input) {
    parser_skip_whitespace(input);
    if (parser_peek(input) != '(') {
        // Entering from a recursive parse call
        // The expression here should be a literal
        // Either a symbol literal, number literal, string literal, char
        // literal, quoted value

        char buf[256] = {0};
        bool contains_letters = false;
        for (int i = 0; parser_peek(input) != ' ' && parser_peek(input) != ')';
             i++) {
            buf[i] = parser_get(input);
            if ('a' <= buf[i] && buf[i] <= 'z' || 'A' <= buf[i] && 'Z' <= 'Z') {
                contains_letters = true;
            }
        }

        if (contains_letters) {
            // Symbol lookup
            assert(false);
        } else {
            // Parse number
            return (Value){
                .tag = NUMBER,
                .val.number = strtod(buf, NULL),
            };
        }
    }

    assert(parser_peek(input) == '(');
    parser_get(input);

    char buf[256] = {0};
    for (int i = 0; parser_peek(input) != ' ' && parser_peek(input) != ')';
         i++) {
        buf[i] = parser_get(input);
    }

    // Check special forms
    if (!strcmp("if", buf)) {
        return handle_if(input);
    } else if (!strcmp("and", buf)) {
        return handle_and(input);
    } else if (!strcmp("or", buf)) {
        return handle_or(input);
    } else if (!strcmp("lambda", buf)) {
        return handle_lambda(input);
    } else if (!strcmp("let", buf)) {
        return handle_let(input);
    }

    // If we make it here the the first symbol must be a function
    // Check if it's a builtin arithmetic operator first
    switch (buf[0]) {
    case '+':
        return handle_add(input);
        break;
    case '-':
        return handle_sub(input);
        break;
    case '*':
        return handle_mul(input);
        break;
    case '/':
        return handle_div(input);
        break;
    case '%':
        return handle_mod(input);
        break;
    }

    // Now we can perform symbol lookup and call the associated procedure
    assert(false);
}

int main(int argc, char *argv[]) {
    /* char buf[4096] = {0}; */
    /* while (true) { */
    /*     printf("jisp> "); */
    /*     fgets(buf, sizeof(buf), stdin); */

    /*     if (buf[0] != '(') { */
    /*         continue; */
    /*     } */

    /*     Parser p = (Parser){.text = buf, .pos = 0}; */

    /*     Value v = parse(&p); */
    /* } */

    {
        Parser p = (Parser){.text = "(+ 1 2)", .pos = 0};

        Value v = parse(&p);
        assert(v.tag == NUMBER && v.val.number == 3.0);
    }

    {
        Parser p = (Parser){.text = "(- 1 2)", .pos = 0};

        Value v = parse(&p);
        assert(v.tag == NUMBER && v.val.number == -1);
    }

    {
        Parser p = (Parser){.text = "(* 1 2)", .pos = 0};

        Value v = parse(&p);
        assert(v.tag == NUMBER && v.val.number == 2);
    }

    {
        Parser p = (Parser){.text = "(/ 1 2)", .pos = 0};

        Value v = parse(&p);
        assert(v.tag == NUMBER && v.val.number == 1. / 2);
    }

    {
        Parser p = (Parser){.text = "(% 1 2)", .pos = 0};

        Value v = parse(&p);
        assert(v.tag == NUMBER && v.val.number == 1);
    }
}
