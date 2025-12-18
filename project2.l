%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct NODE NODE;   
#include "y.tab.h"
%}

%%
"#define"                                          { yylval.string=strdup(yytext); return DEFINE; }
int                                                { yylval.string=strdup(yytext); return INT; }
void                                               { yylval.string=strdup(yytext); return VOID; }
if                                                 { yylval.string=strdup(yytext); return IF; }
else                                               { yylval.string=strdup(yytext); return ELSE; }
for                                                { yylval.string=strdup(yytext); return FOR; }
continue                                           { yylval.string=strdup(yytext); return CONTINUE; }
[a-zA-Z_][a-zA-Z_0-9]*                             { yylval.string=strdup(yytext); return ID; }
"="|"+="                                           { yylval.string=strdup(yytext); return OP_ASSIGN; }
"++"                                               { yylval.string=strdup(yytext); return OP_INC; }
"--"                                               { yylval.string=strdup(yytext); return OP_DEC; }
"+"|"-"                                            { yylval.string=strdup(yytext); return OP_ADD; }
"*"|"/"                                            { yylval.string=strdup(yytext); return OP_MUL; }
"&&"|"||"                                          { yylval.string=strdup(yytext); return OP_LOGIC; }
"!="|"=="|"<"|">"|"<="|">="                        { yylval.string=strdup(yytext); return OP_REL; }
0[xX][0-9a-fA-F]+                                  { yylval.string=strdup(yytext); return NUM_HEX; }
0[bB][01]+                                         { yylval.string=strdup(yytext); return NUM_BIN; }
[0-9]+                                             { yylval.string=strdup(yytext); return NUM; }
"("                                                { yylval.string=strdup(yytext); return LPAREN; }
")"                                                { yylval.string=strdup(yytext); return RPAREN; }
"{"                                                { yylval.string=strdup(yytext); return LBRACE; }
"}"                                                { yylval.string=strdup(yytext); return RBRACE; }
"["                                                { yylval.string=strdup(yytext); return LBRACKET; }
"]"                                                { yylval.string=strdup(yytext); return RBRACKET; }
","                                                { yylval.string=strdup(yytext); return COMMA; }
";"                                                { yylval.string=strdup(yytext); return SEMICOLON; }
\n                                                 { yylineno++; }
\/\/.*|\/\*([^*]|\*+[^/])*\*\/                     /* skip comments */
[ \t\r]+                                           /* skip whitespace */
%% 
