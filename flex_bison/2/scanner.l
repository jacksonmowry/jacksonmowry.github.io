%option noyywrap

%{
int chars=0;
int words=0;
int lines=0;

int total_chars=0;
int total_words=0;
int total_lines=0;
%}

%%
[a-zA-Z]+	{ words++; chars += strlen(yytext); }
\n		{ chars++; lines++; }
.		{ chars++; }

%%

int main(int argc, char* argv[]) {
	if (argc < 2) {
		yylex();
		printf("%8d%8d%8d\n", lines, words, chars);
		return 0;
	}

	for (int i = 1; i < argc; i++) {
		FILE* f = fopen(argv[i], "r");

		if (!f) {
			perror(argv[i]);
			return 1;
		}

		yyrestart(f);
		yylex();
		fclose(f);
		printf("%8d%8d%8d %s\n", lines, words, chars, argv[i]);
		total_chars += chars;
		total_words += words;
		total_lines += lines;

		chars=0;
		words=0;
		lines=0;
	}

	if (argc > 1) {
		printf("%8d%8d%8d total\n", total_lines, total_words, total_chars);
	}

	return 0;
}
