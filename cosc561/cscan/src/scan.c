#include "scan.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lookahead;
char* buf;
size_t buf_len;

void consume_whitespace() {
    while (isspace(lookahead)) {
        lookahead = getchar();
    }
}

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
top:
    consume_whitespace();
    switch (lookahead) {
    case EOF: {
        return (Token){.type = EOF_TOKEN, .lexeme = "EOF"};
    } break;
    case '|': {
        match('|');
        switch (lookahead) {
        case '|': {
            // Handle logical or
            match('|');
            return (Token){.type = OR, .lexeme = "||"};
        } break;
        case '=': {
            // Handle or equals
            match('=');
            return (Token){.type = ASSIGN_OR, .lexeme = "|="};
        } break;
        default: {
            // Handle bitwise or
            return (Token){.type = BIT_OR, .lexeme = "|"};
        } break;
        }
    } break;
    case '&': {
        match('&');
        switch (lookahead) {
        case '&': {
            // Handle logical and
            match('&');
            return (Token){.type = AND, .lexeme = "&&"};
        } break;
        case '=': {
            // Handle and equals
            match('=');
            return (Token){.type = ASSIGN_AND, .lexeme = "&="};
        } break;
        default: {
            // Handle bitwise and
            return (Token){.type = BIT_AND, .lexeme = "&"};
        } break;
        }
    } break;
    case '!': {
        match('!');
        switch (lookahead) {
        case '=': {
            // Handle not equals
            match('=');
            return (Token){.type = NE, .lexeme = "!="};
        } break;
        default: {
            // Handle logical not
            return (Token){.type = NOT, .lexeme = "!"};
        } break;
        }
    } break;
    case '~': {
        // Only monograph
        match('~');
        return (Token){.type = BIT_NOT, .lexeme = "~"};
    } break;
    case '^': {
        match('^');
        switch (lookahead) {
        case '=': {
            // Handle xor equals
            match('=');
            return (Token){.type = ASSIGN_XOR, .lexeme = "^="};
        } break;
        default: {
            // Handle xor
            return (Token){.type = BIT_XOR, .lexeme = "^"};
        } break;
        }
    } break;
    case '>': {
        match('>');
        switch (lookahead) {
        case '>': {
            match('>');
            switch (lookahead) {
            case '=': {
                // Handle rshift equals
                match('=');
                return (Token){.type = ASSIGN_RSH, .lexeme = ">>="};
            } break;
            default: {
                // Handle rshift
                return (Token){.type = RSH, .lexeme = ">>"};
            } break;
            }
        } break;
        case '=': {
            // Handle gt or equals
            match('=');
            return (Token){.type = GE, .lexeme = ">="};
        } break;
        default: {
            // Handle gt
            return (Token){.type = GT, .lexeme = ">"};
        } break;
        }
    } break;
    case '<': {
        match('<');
        switch (lookahead) {
        case '<': {
            match('<');
            switch (lookahead) {
            case '=': {
                // Handle lshift equals
                match('=');
                return (Token){.type = ASSIGN_LSH, .lexeme = "<<="};
            } break;
            default: {
                // Handle lshift
                return (Token){.type = LSH, .lexeme = "<<"};
            } break;
            }
        } break;
        case '=': {
            // Handle lt or equals
            match('=');
            return (Token){.type = LE, .lexeme = "<="};
        } break;
        default: {
            // Handle lt
            return (Token){.type = LT, .lexeme = "<"};
        } break;
        }
    } break;
    case '=': {
        match('=');
        switch (lookahead) {
        case '=': {
            match('=');
            return (Token){.type = EQ, .lexeme = "=="};
        } break;
        default: {
            return (Token){.type = ASSIGN, .lexeme = "="};
        } break;
        }
    } break;
    case '+': {
        match('+');
        switch (lookahead) {
        case '=': {
            match('=');
            return (Token){.type = ASSIGN_ADD, .lexeme = "+="};
        } break;
        default: {
            return (Token){.type = ADD, .lexeme = "+"};
        } break;
        }
    } break;
    case '-': {
        match('-');
        switch (lookahead) {
        case '=': {
            match('=');
            return (Token){.type = ASSIGN_SUB, .lexeme = "-="};
        } break;
        default: {
            return (Token){.type = SUB, .lexeme = "-"};
        } break;
        }
    } break;
    case '*': {
        match('*');
        switch (lookahead) {
        case '=': {
            match('=');
            return (Token){.type = ASSIGN_MUL, .lexeme = "*="};
        } break;
        default: {
            return (Token){.type = MUL, .lexeme = "*"};
        } break;
        }
    } break;
    case '%': {
        match('%');
        switch (lookahead) {
        case '=': {
            match('=');
            return (Token){.type = ASSIGN_MOD, .lexeme = "%="};
        } break;
        default: {
            return (Token){.type = MOD, .lexeme = "%"};
        } break;
        }
    } break;
    case '/': {
        match('/');
        switch (lookahead) {
        case '/': {
            // Handle comment
            match('/');
            while (lookahead != '\n') {
                lookahead = getchar();
            }
            // Lookahead is now at the end of the line
            // One last getchar to move to the beginning of the next line
            assert(lookahead == '\n');
            lookahead = getchar();

            goto top;
        } break;
        case '*': {
            // Handle multi-line comment
            match('*');

            // We now need to consume characters until we find '*/'
            char buf[2] = {0};
            size_t idx = 2;
            buf[0] = lookahead;
            lookahead = getchar();
            buf[1] = lookahead;

            while (buf[(idx - 2) % 2] != '*' || buf[(idx - 1) % 2] != '/') {
                buf[idx % 2] = lookahead;
                lookahead = getchar();
                idx++;
            }

            assert(buf[(idx - 2) % 2] == '*' && buf[(idx - 1) % 2] == '/');
            goto top;
        } break;
        case '=': {
            // Handle div equals
            match('=');
            return (Token){.type = ASSIGN_DIV, .lexeme = "/="};
        } break;
        default: {
            return (Token){.type = DIV, .lexeme = "/"};
        } break;
        }
    } break;
    case ';': {
        // Only monograph
        match(';');
        return (Token){.type = SEMICOLON, .lexeme = ";"};
    } break;
    case ',': {
        // Only monograph
        match(',');
        return (Token){.type = COMMA, .lexeme = ","};
    } break;
    case ':': {
        // Only monograph
        match(':');
        return (Token){.type = COLON, .lexeme = ":"};
    } break;
    case '(': {
        // Only monograph
        match('(');
        return (Token){.type = LPAREN, .lexeme = "("};
    } break;
    case ')': {
        // Only monograph
        match(')');
        return (Token){.type = RPAREN, .lexeme = ")"};
    } break;
    case '[': {
        // Only monograph
        match('[');
        return (Token){.type = LBRACE, .lexeme = "["};
    } break;
    case ']': {
        // Only monograph
        match(']');
        return (Token){.type = RBRACE, .lexeme = "]"};
    } break;
    case '{': {
        // Only monograph
        match('{');
        return (Token){.type = LBRACKET, .lexeme = "{"};
    } break;
    case '}': {
        // Only monograph
        match('}');
        return (Token){.type = RBRACKET, .lexeme = "}"};
    } break;
    case '"': {
        buf_len = 0;

        match('"');

        while (lookahead != '"') {
            if (lookahead == '\\') {
                // Handle escaped char
                match('\\');

                switch (lookahead) {
                case 'n':
                    buf[buf_len++] = '\n';
                    break;
                case 't':
                    buf[buf_len++] = '\t';
                    break;
                case 'r':
                    buf[buf_len++] = '\r';
                    break;
                case 'v':
                    buf[buf_len++] = '\v';
                    break;
                case 'f':
                    buf[buf_len++] = '\f';
                    break;
                case 'a':
                    buf[buf_len++] = '\a';
                    break;
                case 'b':
                    buf[buf_len++] = '\b';
                    break;
                case '\\':
                    buf[buf_len++] = '\\';
                    break;
                case '"':
                    buf[buf_len++] = '\"';
                    break;
                case '\'':
                    buf[buf_len++] = '\'';
                    break;
                case '0':
                    buf[buf_len++] = '\0';
                    break;
                }

                lookahead = getchar();
                continue;
            }

            buf[buf_len++] = (char)lookahead;
            lookahead = getchar();
        }

        assert(lookahead == '"');
        match('"');

        buf[buf_len] = '\0';

        return (Token){.type = STRING_LITERAL, .lexeme = strdup(buf)};
    } break;
    default: {
        // Parsing all other tokens
        // Should be identifiers, keywords, and int/float literals
        buf_len = 0;

        if (isalpha(lookahead)) {
            // Parse identifier or keyword
            buf[buf_len++] = lookahead;
            lookahead = getchar();

            while (isalnum(lookahead)) {
                buf[buf_len++] = lookahead;
                lookahead = getchar();
            }

            assert(!isalnum(lookahead));
            buf[buf_len] = '\0';

            // Now attempt to match keywords
            if (!strncmp("char", buf, 4)) {
                return (Token){.type = CHAR, .lexeme = "char"};
            } else if (!strncmp("int", buf, 3)) {
                return (Token){.type = INT, .lexeme = "int"};
            } else if (!strncmp("float", buf, 5)) {
                return (Token){.type = FLOAT, .lexeme = "float"};
            } else if (!strncmp("double", buf, 6)) {
                return (Token){.type = DOUBLE, .lexeme = "double"};
            } else if (!strncmp("if", buf, 2)) {
                return (Token){.type = IF, .lexeme = "if"};
            } else if (!strncmp("else", buf, 4)) {
                return (Token){.type = ELSE, .lexeme = "else"};
            } else if (!strncmp("while", buf, 5)) {
                return (Token){.type = WHILE, .lexeme = "while"};
            } else if (!strncmp("do", buf, 2)) {
                return (Token){.type = DO, .lexeme = "do"};
            } else if (!strncmp("for", buf, 3)) {
                return (Token){.type = FOR, .lexeme = "for"};
            } else if (!strncmp("return", buf, 6)) {
                return (Token){.type = RETURN, .lexeme = "return"};
            } else if (!strncmp("break", buf, 5)) {
                return (Token){.type = BREAK, .lexeme = "break"};
            } else if (!strncmp("continue", buf, 8)) {
                return (Token){.type = CONTINUE, .lexeme = "continue"};
            } else if (!strncmp("goto", buf, 4)) {
                return (Token){.type = GOTO, .lexeme = "goto"};
            } else {
                // Just an identifier
                return (Token){.type = IDENTIFIER, .lexeme = strdup(buf)};
            }
        } else if (isdigit(lookahead)) {
            // Parse number, we don't yet know if this is an int or float
            bool contains_dot = false;
            buf[buf_len++] = lookahead;
            lookahead = getchar();

            while (isdigit(lookahead) || lookahead == '.') {
                if (lookahead == '.' && contains_dot) {
                    error();
                } else if (lookahead == '.') {
                    contains_dot = true;
                }

                buf[buf_len++] = lookahead;
                lookahead = getchar();
            }

            buf[buf_len] = '\0';

            if (contains_dot) {
                // real literal
                double val = strtod(buf, NULL);
                return (Token){.type = REAL_LITERAL, .float_literal = val};
            } else {
                // Integer literal
                long int val = strtoll(buf, NULL, 0);
                return (Token){.type = INT_LITERAL, .int_literal = val};
            }
        } else {
            error();
        }
    } break;
    }

    assert(false);
}
