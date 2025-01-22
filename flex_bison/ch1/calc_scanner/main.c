#include "fb1-5.tab.h"
#include <stdio.h>
#include <stdlib.h>

extern FILE* yyin;

int main(int argc, char* argv[]) {
	if (argc == 2) {
		if (!(yyin = fopen(argv[1], "r"))) {
			perror(argv[1]);
			return EXIT_FAILURE;
		}
	}	

	return yyparse();
}
