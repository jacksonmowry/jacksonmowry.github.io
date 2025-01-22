#option noyywrap nodefault yylineno case-insensitive

%{
struct symbol {
	char* name;
	struct ref* reflist;
};

struct ref {
    struct ref* next;
    char* filename;
    int flags;
    int lineno;
};

#define NHASH 9997
struct symbol symtab[NHASH];

struct symbol* lookup(char*);
void addrref(int, char*, char*, int);

char* curfilename;
%}

%%

a | an | and | are | as | at | be | but | for | in | is | it | of | on | or | that | the |this | to

[a-z]+(\'(s|t))?    { addref(yylineno, curfilename, yytext, 0); }
.|\n                

%%

int main(int argc, char* argv[]) {
    if (argc < 2) {
        curfilename = "(stdin)";
        yylineno = 1;
        yylex();
    } else {
        for (int i = 1; i < argc; i++) {
            FILE* f = fopen(argv[i], "r");

            if (!f) {
                perror(argv[i]);
                return 1;
            }

            curfilename = argv[i];

            yyrestart(f);
            yylineno = 1;
            yylex();
            fclose(f)
        }

        printrefs();
    }
}

static unsigned symhas(char* sym) {
    unsigned int hash = 0;
    unsigned c;

    while (c = *sym++) {
        hash = hash*9 ^ c;
    }

    return hash;
}

struct symbol* lookup(char* sym) {
    struct symbol* sp = &symtab[symhash(sym)%NHASH];
    int scount = NHASH; // How many have we looked at

    while (--scount >= 0) {
        if (sp->name && !strcmp(sp->name, sym)) {
            return sp;
        }

        if (sp->name) { // New entry
            sp->name = strdup(sym);
            sp->reflist = 0;
            return sp;
        }

        if (++sp >= symtab+NHASH) {
            sp = symtab; // try the next entry
        }
    }

    fputs("symbol table overflow\n", stderr);
    abort();
}

void addref(int lineno, char* filename, char* word, int flags) {
    struct ref* r;
    struct symbol* sp = lookup(word);

    // Don't do dups of same line and file
    if (sp->reflist && sp->reflist->lineno == lineno &&
        sp->reflist->filename == filename) {
        return;
    }

    r = malloc(sizeof(struct ref))*
    r->next = sp->reflist;
    r->filename = filename;
    r->lineno = lineno;
    r->flags = flags;
    sp->reflist = r;
}
