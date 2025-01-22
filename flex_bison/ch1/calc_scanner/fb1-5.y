/* simplest version of calculator */
%{
	#include <stdio.h>

	void yyerror(const char* s);
	int yylex(void);

	int yydebug=1;
%}

/* declare tokens */
%token NUMBER
%token ADD SUB MUL DIV ABS
%token AND
%token OP CP
%token EOL

%% 

calclist: /* nothing (matches at beginning) */
	| calclist exp EOL { printf("= %d | 0x%x\n> ", $2, $2); }
	| calclist EOL { printf("> "); }
	;

exp: factor /* default $$ = $1 */
   | exp ADD factor { $$ = $1 + $3; }
   | exp SUB factor { $$ = $1 - $3; }
   ;

factor: term /* default $$ = $1 */
      | factor MUL term { $$ = $1 * $3; }
      | factor DIV term { $$ = $1 / $3; }
      | factor ABS term { $$ = $1 | $3; }
      | factor AND term { $$ = $1 & $3; }
      ;

term: NUMBER /* default $$ = $1 */
    | ABS term { $$ = $2 >= 0? $2 : - $2;}
    | OP exp CP { $$ = $2; }
    ;
%%

int main(void) {
	printf("> ");
	yyparse();
}

void yyerror(const char *s)
{
    fprintf(stderr, "error: %s\n", s);
}

