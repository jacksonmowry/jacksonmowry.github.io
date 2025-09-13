#include "scan.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    Token cur_token;

    init_scanner();

    cur_token = get_token();
    while (cur_token.type != EOF_TOKEN) {
        print_token(cur_token);
        if (cur_token.type == STRING_LITERAL || cur_token.type == IDENTIFIER) {
            free(cur_token.lexeme);
        }
        cur_token = get_token();
    }
}
