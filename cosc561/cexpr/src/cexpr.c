#include "scan.h"

#include <assert.h>
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
    PREC_NEGATE,
    PREC_ERROR
} Precedence;

Precedence precedence(Token t) {
    switch (t.type) {
    case ASSIGN:
    case ASSIGN_OR:
    case ASSIGN_XOR:
    case ASSIGN_AND:
    case ASSIGN_LSH:
    case ASSIGN_RSH:
    case ASSIGN_ADD:
    case ASSIGN_SUB:
    case ASSIGN_MUL:
    case ASSIGN_DIV:
    case ASSIGN_MOD:
        return PREC_ASSIGN;
    case BIT_OR:
        return PREC_BIT_OR;
    case BIT_XOR:
        return PREC_BIT_XOR;
    case BIT_AND:
        return PREC_BIT_AND;
    case LSH:
    case RSH:
        return PREC_SHIFT;
    case ADD:
    case SUB:
        return PREC_ADD_SUB;
    case MUL:
    case DIV:
    case MOD:
        return PREC_MUL_DIV_MOD;
    default:
        fprintf(stderr, "Token type %s cannot be used as a binary operator\n",
                t.lexeme);
        assert(false);
    }
}

typedef enum Associativity {
    ASSOC_RIGHT,
    ASSOC_LEFT,
    ASSOC_ERROR
} Associativity;

Associativity associativity(Token t) {
    switch (t.type) {
    case ASSIGN:
    case ASSIGN_OR:
    case ASSIGN_XOR:
    case ASSIGN_AND:
    case ASSIGN_LSH:
    case ASSIGN_RSH:
    case ASSIGN_ADD:
    case ASSIGN_SUB:
    case ASSIGN_MUL:
    case ASSIGN_DIV:
    case ASSIGN_MOD:
        return ASSOC_RIGHT;
    case BIT_NOT:
        return ASSOC_RIGHT;
    case BIT_OR:
    case BIT_XOR:
    case BIT_AND:
        return ASSOC_LEFT;
    case LSH:
    case RSH:
        return ASSOC_LEFT;
    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case MOD:
        return ASSOC_LEFT;
    default:
        fprintf(stderr,
                "Token type %s does not have an associated associtivity\n",
                t.lexeme);
        assert(false);
    }
}

typedef struct Variable {
    char* name;
    long long val;
    struct Variable* next;
} Variable;

Variable* get_or_create_variable(char* name);
Variable* set_variable(char* name, long long value);

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
    bool error;
} Value;

static Token lookahead_token;
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
    if (lookahead_token.type == type) {
        lookahead_token = get_next_token();
    } else {
        token_error(lookahead_token);
    }
}

void init_lookahead_token() {
    peek = null_token();
    lookahead_token = get_next_token();
}

bool in_first_of_expr(Token t) {
    return (t.type == LPAREN || t.type == SUB || t.type == BIT_NOT ||
            t.type == IDENTIFIER || t.type == INT_LITERAL);
}

bool is_arithmetic_operator(Token t) {
    return (t.type == ADD || t.type == SUB || t.type == MUL || t.type == DIV ||
            t.type == MOD || t.type == BIT_OR || t.type == BIT_XOR ||
            t.type == BIT_AND || t.type == LSH || t.type == RSH);
}

bool is_assignment_operator(Token t) {
    switch (t.type) {
    case ASSIGN:
    case ASSIGN_OR:
    case ASSIGN_XOR:
    case ASSIGN_AND:
    case ASSIGN_LSH:
    case ASSIGN_RSH:
    case ASSIGN_ADD:
    case ASSIGN_SUB:
    case ASSIGN_MUL:
    case ASSIGN_DIV:
    case ASSIGN_MOD:
        return true;
    default:
        return false;
    }
}

Value apply_arithmetic_operator(Token op, Value lhs, Value rhs) {
    Value v = (Value){.type = RVAL};

    // Both lhs and rhs could be variable, let's resolve their values
    if (lhs.type == LVAL) {
        lhs.rval = lhs.lval->val;
    }
    if (rhs.type == LVAL) {
        rhs.rval = rhs.lval->val;
    }

    if ((op.type == DIV || op.type == MOD) && rhs.rval == 0) {
        // We need to do something here to A) not return a value and B) skip the
        // computation
        printf("error: attempting to divide by zero\n");
        lhs.error = true;
        return lhs;
    }

    switch (op.type) {
    case ADD:
        if ((lhs.rval > 0 && rhs.rval > 0 && (lhs.rval + rhs.rval) < 0) ||
            (lhs.rval < 0 && rhs.rval < 0 && (lhs.rval + rhs.rval) > 0)) {
            printf("error: integer overflow (add)\n");
            lhs.error = true;
            return lhs;
        }
        v.rval = lhs.rval + rhs.rval;
        break;
    case SUB:
        if ((lhs.rval > 0 && -rhs.rval > 0 && (lhs.rval + -rhs.rval) < 0) ||
            (lhs.rval < 0 && -rhs.rval < 0 && (lhs.rval + -rhs.rval) > 0)) {
            printf("error: integer overflow (sub)\n");
            lhs.error = true;
            return lhs;
        }
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
    case BIT_OR:
        v.rval = lhs.rval | rhs.rval;
        break;
    case BIT_XOR:
        v.rval = lhs.rval ^ rhs.rval;
        break;
    case BIT_AND:
        v.rval = lhs.rval & rhs.rval;
        break;
    case LSH:
        // TODO error check
        v.rval = lhs.rval << rhs.rval;
        break;
    case RSH:
        // TODO error check
        v.rval = lhs.rval >> rhs.rval;
        break;
    default:
        printf("operator token type: %s not implemented\n",
               token_type_str(op.type));
        exit(1);
    }

    return v;
}

Value apply_assignment_operator(Token op, Value lhs, Value rhs) {
    // Pre-resolve the rval of rhs
    long long r;
    if (rhs.type == LVAL) {
        r = rhs.lval->val;
    } else {
        r = rhs.rval;
    }
    switch (op.type) {
    case ASSIGN:
        set_variable(lhs.lval->name, r);
        break;
    case ASSIGN_OR:
        set_variable(lhs.lval->name, lhs.lval->val | r);
        break;
    case ASSIGN_XOR:
        set_variable(lhs.lval->name, lhs.lval->val ^ r);
        break;
    case ASSIGN_AND:
        set_variable(lhs.lval->name, lhs.lval->val & r);
        break;
    case ASSIGN_LSH:
        set_variable(lhs.lval->name, lhs.lval->val << r);
        break;
    case ASSIGN_RSH:
        set_variable(lhs.lval->name, lhs.lval->val >> r);
        break;
    case ASSIGN_ADD:
        set_variable(lhs.lval->name, lhs.lval->val + r);
        break;
    case ASSIGN_SUB:
        set_variable(lhs.lval->name, lhs.lval->val - r);
        break;
    case ASSIGN_MUL:
        set_variable(lhs.lval->name, lhs.lval->val * r);
        break;
    case ASSIGN_DIV:
        set_variable(lhs.lval->name, lhs.lval->val / r);
        break;
    case ASSIGN_MOD:
        set_variable(lhs.lval->name, lhs.lval->val % r);
        break;
    default:
        fprintf(stderr, "Not an assignment operator %s\n", op.lexeme);
        break;
    }

    printf("$> %s := %lld\n", lhs.lval->name, lhs.lval->val);

    return lhs;
}

Value expr();
Value __expr(Precedence);

Value term() {

    if (lookahead_token.type == INT_LITERAL) {
        Value v = (Value){.type = RVAL, .rval = lookahead_token.int_literal};

        match(lookahead_token.type);

        return v;
    } else if (lookahead_token.type == LPAREN) {
        match(LPAREN);

        Value v = expr();

        match(RPAREN);

        return v;
    } else if (lookahead_token.type == SUB) {
        match(SUB);

        Value v = term();
        v.rval = -v.rval;

        return v;
    } else if (lookahead_token.type == BIT_NOT) {
        match(BIT_NOT);

        Value v = term();
        v.rval = ~v.rval;

        return v;
    } else {
        Variable* v = get_or_create_variable(lookahead_token.lexeme);
        match(IDENTIFIER);

        Value val = (Value){.type = LVAL, .lval = v};
        return val;
    }

    token_error(lookahead_token);
    assert(false);
}

Value __expr(Precedence min_prec) {
    Token op;
    Value lhs, rhs;
    Precedence next_min;

    lhs = term();

    if (is_arithmetic_operator(lookahead_token)) {
        while (is_arithmetic_operator(lookahead_token) &&
               precedence(lookahead_token) >= min_prec) {
            op = lookahead_token;
            match(op.type);
            if (associativity(op) == ASSOC_LEFT) {
                next_min = precedence(op) + 1;
            } else {
                next_min = precedence(op);
            }

            rhs = __expr(next_min);
            lhs = apply_arithmetic_operator(op, lhs, rhs);

            if (lhs.error) {
                return lhs;
            }
        }
    } else if (is_assignment_operator(lookahead_token)) {
        if (lhs.type != LVAL) {
            fprintf(stderr, "error: cannot assign to rval: %lld\n", lhs.rval);
        }

        op = lookahead_token;
        match(op.type);

        rhs = expr();

        lhs = apply_assignment_operator(op, lhs, rhs);
        assert(lhs.type == LVAL);

        return lhs;
    }

    if (lhs.type == LVAL) {
        lhs.type = RVAL;
        lhs.rval = lhs.lval->val;
    }
    return lhs;
}

Value expr() { return __expr(PREC_MIN); }

void stmt() {
    Value val = (Value){.rval = 0ll, .type = RVAL};

    // Are we in the set of symbols that can start an expression
    if (in_first_of_expr(lookahead_token)) {
        val = expr();
    }

    // Check if this is valid before printing
    if (lookahead_token.type != SEMICOLON) {
        token_error(lookahead_token);
    }

    if (val.type == RVAL && !val.error) {
        printf("$> %lld\n", val.rval);
    }
    // We either had an expression or simply just a seimicolon
    match(SEMICOLON);
}

Variable* variables = NULL;

Variable* get_or_create_variable(char* name) {
    Variable* prev = NULL;
    Variable* head = variables;

    while (head != NULL) {
        if (!strcmp(name, head->name)) {
            // Found the match
            return head;
        }

        prev = head;
        head = head->next;
    }

    // If we get here it does not yet exist
    Variable* new = calloc(sizeof(Variable), 1);

    new->name = name;

    if (variables == NULL) {
        variables = new;
    } else {
        prev->next = new;
    }

    return new;
}

Variable* set_variable(char* name, long long value) {
    Variable* v = get_or_create_variable(name);

    v->val = value;

    return v;
}

int main() {
    init_scanner();
    init_lookahead_token();

    while (lookahead_token.type != EOF_TOKEN) {
        stmt();
    }

    while (variables) {
        Variable* tmp = variables->next;
        free(variables);
        variables = tmp;
    }
}
