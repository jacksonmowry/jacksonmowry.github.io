%{
// Work recognizer with a symbol table.

enum {
    LOOKUP=0, // Default rether than defining
    VERB,
    ADJ,
    ADV,
    NOUN,
    PREP,
    PRON,
    CONJ
};

int state;

int add_word(int type, char* word);
int lookup_word(char* word);
%}

%option noyywrap

%%
\n { state = LOOKUP; } /* End of line, return to default state */

   /* Whenever a line starts with a reserved part of speech name
      start defining words of that type */
^verb { state = VERB; }
^adj { state = ADJ; }
^adv { state = ADV; }
^noun { state = NOUN; }
^prep { state = PREP; }
^pron { state = PRON; }
^conj { state = CONJ; }

[a-zA-Z]+ {
            /* A normal word, define it or look it up */
          if (state != LOOKUP) {
             add_word(state, yytext);
          } else {
            switch (lookup_word(yytext)) {
                   case VERB: printf("%s: verb\n", yytext); break;
                   case ADJ: printf("%s: adj\n", yytext); break;
                   case ADV: printf("%s: adv\n", yytext); break;
                   case NOUN: printf("%s: noun\n", yytext); break;
                   case PREP: printf("%s: prep\n", yytext); break;
                   case PRON: printf("%s: pron\n", yytext); break;
                   case CONJ: printf("%s: conj\n", yytext); break;
                   default:
                        printf("%s: don't recognize\n", yytext);
                        break;
            }
          }
}
. // ignore everything else

%%

int main() {
    yylex();
}

struct word {
       char *word_name;
       int word_type;
       struct word* next;
};

struct word* word_list; // First element in word list

extern void* malloc();

int add_word(int type, char* word) {
    struct word* wp;

    if (lookup_word(word) != LOOKUP) {
       printf("!!! warning: word %s alread defined\n", word);
       return 0;
    }

    // Word was not in our list, allocate a new entry and link it on the list

    wp = (struct word*)malloc(sizeof(struct word));

    wp->next = word_list;

    // Have to copy the word itelf as well
    wp->word_name = (char*)malloc(strlen(word)+1);
    strcpy(wp->word_name, word);
    wp->word_type = type;
    word_list = wp;
    return 1; // it worked
}

int lookup_word(char* word) {
    struct word* wp = word_list;

    // Search list for match
    for (; wp; wp = wp->next) {
        if (!strcmp(wp->word_name, word)) {
           return wp->word_type;
        }
    }

    return LOOKUP; // Not found
}
