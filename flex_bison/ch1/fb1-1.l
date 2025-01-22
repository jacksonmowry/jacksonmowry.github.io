/* Just like Unix wc */
%{
int chars = 0;
int words = 0;
int lines = 0;
%}

%%

[a-zA-Z]+ 		{ words++; chars += strlen(yytext); }
[^ \t\n\r\f\v]+		{ chars++; lines++; }
.			{ chars++; }

%%

int main(int argc, char** argv) {
	yylex();
	printf("%8d%8d%8d\n", lines,words,chars);
}
