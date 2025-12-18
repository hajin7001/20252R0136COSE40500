%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>
void to_uppercase(char *s);   
%}

%%
0[xX][A-Fa-f0-9]+ printf("NUM_HEX ");

0b[01]+ printf("NUM_BIN ");

-?0|-?[1-9][0-9]* printf("NUM ");

int|void|if|else|for|continue {
    to_uppercase(yytext);
    printf("%s ", yytext);
}

\#define printf("DEFINE ");

[a-zA-Z_][a-zA-Z0-9_]* printf("ID ");

"!="|"=="|"<"|"<="|">"|">=" printf("OP_REL ");

"="|"+=" printf("OP_ASSIGN ");

"++" printf("OP_INC ");

"--" printf("OP_DEC ");

"+" printf("OP_ADD ");

"-" printf("OP_SUB ");

"*" printf("OP_MUL ");

"/" printf("OP_DIV ");

"&&"|"||" printf("OP_LOGIC ");

"//".* printf("COMMENT ");
"/*"([^*]|\n|\*+[^/])*"*/" printf("COMMENT ");

"["|"]"|"("|")"|"{"|"}"|";"|"," printf("%s ", yytext);

\n printf("\n");
[ \t]+ /* eat up whitespace */

. /* unrecognized character */

%%
void to_uppercase(char *s) {
    for (int i = 0; s[i]; i++) {
        s[i] = toupper((unsigned char)s[i]);
    }
}

int main( argc, argv )
int argc;
char **argv;
{
    ++argv, --argc; /* skip over program name */
    if ( argc > 0 )
        yyin = fopen( argv[0], "r" );
    else
        yyin = stdin;
    yylex();
}