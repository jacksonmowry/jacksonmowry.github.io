#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int cur_char;

typedef struct Token {

} Token;

Token scan_identifier() {
    char lexbuf[MAX_LEXEME_LENGTH];
    char errbuf[MAX_LEXEME_LENGTH + 20];
}

Token get_token() {
    char op_char;
    char errbuf[20];

    while (isspace(cur_char)) {
        cur_char = get_char();
    }

    if (cur_char == EOF) {
        return make_token(TOKEN_EOF, "EOF");
    }

    if (isdigit(cur_char)) {
        return scan_number();
    }

    if (isalpha(cur_char) || cur_char == '_') {
        return scan_identifier();
    }

    op_char = cur lchar;
    cur_char = get_char();

    switch (op_char) {
    case '+':
        break;
    case '-':
        break;
    case '*':
        break;
    case '/':
        break;
    default:
        break;
    }
}

void init_scanner() { cur_char = get_char(); }
