%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"   /* tokens e yylval do Bison */
%}

%option noinput nounput
%x COMMENT

%%

true                    { yylval.boolValue = 1; return BOOL_LIT; }
false                   { yylval.boolValue = 0; return BOOL_LIT; }

[0-9]+"."[0-9]+         { yylval.floatValue = strtod(yytext, NULL); return FLOAT_LIT; }
[0-9]+                  { yylval.intValue = atoi(yytext); return INT_LIT; }

\"([^\\\n]|(\\.))*\"    {
                          /* Remove aspas e guarda string */
                          size_t len = yyleng - 2;
                          char *str = (char*)malloc(len + 1);
                          strncpy(str, yytext + 1, len);
                          str[len] = '\0';
                          yylval.str = str;
                          return STRING_LIT;
                        }

"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"for"                   { return FOR; }
"function"              { return FUNCTION; }
"return"                { return RETURN; }

[a-zA-Z_][a-zA-Z0-9_]*  { yylval.str = strdup(yytext); return IDENT; }

[ \t\n]+                { /* ignora espaços, tabs e quebras de linha */ }

"//".*                  { /* ignora comentários de linha única */ }
"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>.|\n           { /* ignora conteúdo de comentário */ }
<COMMENT><<EOF>>        {
                          fprintf(stderr, "Erro: Comentário multilinha não foi fechado.\n");
                          yyterminate();
                        }

"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return TIMES; }
"/"                     { return DIVIDE; }

"=="                    { return EQ; }
"!="                    { return NEQ; }
"<"                     { return LT; }
">"                     { return GT; }
"<="                    { return LE; }
">="                    { return GE; }

"("                     { return LPAREN; }
")"                     { return RPAREN; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
","                     { return COMMA; }
";"                     { return SEMICOLON; }

"&&"                    { return AND; }
"||"                    { return OR; }
"!"                     { return NOT; }
"="                     { return ASSIGN; }

.                       { fprintf(stderr, "Caractere não reconhecido: %s\n", yytext); }

%%

int yywrap(void) { return 1; }
