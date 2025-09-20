#include "scan.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum Precedence {
    PREC_MIN,
    PREC_ASSIGN,
    PREC_BIT_OR,
    PREC_BIT_XOR,
    PREC_BIT_AND,
    PREC_SHIFT,
    PREC_ADD_SUB,
    PREC_MUL_DIV_MOD,
    PREC_ERROR
} Precedence;

typedef enum Associativity {
    ASSOC_RIGHT,
    ASSOC_LEFT,
    ASSOC_ERROR
} Associativity;

typedef struct Variable {
    char* name;
    long long val;
    struct Variable* next;
} Variable;

enum ValueType {
    LVAL,
    RVAL,
};

typedef struct Value {
    enum ValueType type;
    union {
        struct Variable* lval;
        long long rval;
    };
} Value;

static Token lookahead;
Token peek;

void token_error(Token t) {
    fprintf(stderr, "error: invalid expression on `%s` token on line: %d:%d\n",
            t.lexeme, t.line, t.column);
    exit(1);
}

Token null_token() {
    Token token;

    token.lexeme = NULL;
    token.type = NULL_TOKEN;
    token.line = 0;
    token.column = 0;

    return token;
}

Token get_next_token() {
    Token next_token;

    if (peek.type != NULL_TOKEN) {
        next_token = peek;
        peek = null_token();
    } else {
        next_token = get_token();
    }

    return next_token;
}

Token peek_token() {
    if (peek.type == NULL_TOKEN) {
        peek = get_token();
    } else {
        fprintf(stderr, "cannot peek more than one token\n");
        exit(1);
    }
    return peek;
}

static void match(TokenType type) {
    if (lookahead.type == type) {
        lookahead = get_next_token();
    } else {
        token_error(lookahead);
    }
}

void init_lookahead() {
    peek = null_token();
    lookahead = get_next_token();
}

bool in_first_of_expr(Token t) {
    return (t.type == LPAREN || t.type == SUB || t.type == BIT_NOT ||
            t.type == IDENTIFIER || t.type == INT_LITERAL);
}

bool is_arithmetic_operator(Token t) {
    return (t.type == ADD || t.type == SUB || t.type == MUL || t.type == DIV ||
            t.type == MOD);
}

Value apply_arithmetic_operator(Token op, Value lhs, Value rhs) {
    Value v = (Value){.type = RVAL};

    if ((op.type == DIV || op.type == MOD) && rhs.rval == 0) {
        printf("warning: attempting to divide by zero\n");
    }

    switch (op.type) {
    case ADD:
        v.rval = lhs.rval + rhs.rval;
        break;
    case SUB:
        v.rval = lhs.rval - rhs.rval;
        break;
    case MUL:
        v.rval = lhs.rval * rhs.rval;
        break;
    case DIV:
        v.rval = lhs.rval / rhs.rval;
        break;
    case MOD:
        v.rval = lhs.rval % rhs.rval;
        break;
    default:
        printf("operator token type: %s not implemented\n",
               token_type_str(op.type));
        exit(1);
    }

    return v;
}

Value term() {

    if (lookahead.type == INT_LITERAL) {
        Value v = (Value){.type = RVAL, .rval = lookahead.int_literal};

        match(lookahead.type);

        return v;
    }

    token_error(lookahead);
}

Value expr() {
    // Return value at end
    Token op;
    Value lhs, rhs;

    lhs = term();
    if (is_arithmetic_operator(lookahead)) {
        op = lookahead;
        match(lookahead.type);

        rhs = expr();
        lhs = apply_arithmetic_operator(op, lhs, rhs);
    }

    return lhs;
}

void stmt() {
    Value val = (Value){.rval = 0ll, .type = RVAL};

    // Are we in the set of symbols that can start an expression
    if (in_first_of_expr(lookahead)) {
        val = expr();
    }

    // Check if this is valid before printing
    if (lookahead.type != SEMICOLON) {
        token_error(lookahead);
    }

    printf("$> %lld\n", val.rval);

    // We either had an expression or simply just a seimicolon
    match(SEMICOLON);
}

int main() {
    init_scanner();
    init_lookahead();

    while (lookahead.type != EOF_TOKEN) {
        stmt();
    }
}
