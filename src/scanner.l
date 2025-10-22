%option yylineno
%option noinput nounput
%x COMMENT

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.tab.h"   /* tokens e yylval do Bison */
void yyerror(const char *s); /* declarada em parser.y */
extern int g_parse_errors;   /* contador do parser.y */

int yycolumn = 1; /* coluna atual */


#define ADVANCE_COL  do { yycolumn += yyleng; } while(0) /* Macro para avançar a coluna */
%}

%%

\n                      { yycolumn = 1; } /* quebra de linha: reseta coluna */
[ \t\r]+                { ADVANCE_COL; } /* ignora espaços em branco, espaços e tabs */

true                    { ADVANCE_COL; yylval.boolValue = 1; return BOOL_LIT; }
false                   { ADVANCE_COL; yylval.boolValue = 0; return BOOL_LIT; }

[0-9]+"."[0-9]+         { ADVANCE_COL; yylval.floatValue = strtod(yytext, NULL); return FLOAT_LIT; }
[0-9]+                  { ADVANCE_COL; yylval.intValue = atoi(yytext); return INT_LIT; }

\"([^\\\n]|(\\.))*\"    { /* Remove aspas e guarda string */
                          ADVANCE_COL;
                          size_t len = yyleng - 2;
                          char *str = (char*)malloc(len + 1);
                          strncpy(str, yytext + 1, len);
                          str[len] = '\0';
                          yylval.str = str;
                          return STRING_LIT;
                        }

"if"                    { ADVANCE_COL; return IF; }
"else"                  { ADVANCE_COL; return ELSE; }
"while"                 { ADVANCE_COL; return WHILE; }
"for"                   { ADVANCE_COL; return FOR; }
"function"              { ADVANCE_COL; return FUNCTION; }
"return"                { ADVANCE_COL; return RETURN; }

[a-zA-Z_][a-zA-Z0-9_]*  { ADVANCE_COL; yylval.str = strdup(yytext); return IDENT; }

"//".* { ADVANCE_COL; }
"/*"                    { ADVANCE_COL; BEGIN(COMMENT); }
<COMMENT>"*/"           { ADVANCE_COL; BEGIN(INITIAL); }
<COMMENT>\n             { yycolumn = 1; }
<COMMENT><<EOF>>        {
                          yyerror("Comentário multilinha não foi fechado");
                          yyterminate();
                        }
<COMMENT>.              { ADVANCE_COL; }

"+"                     { ADVANCE_COL; return PLUS; }
"-"                     { ADVANCE_COL; return MINUS; }
"*"                     { ADVANCE_COL; return TIMES; }
"/"                     { ADVANCE_COL; return DIVIDE; }

"=="                    { ADVANCE_COL; return EQ; }
"!="                    { ADVANCE_COL; return NEQ; }
"<"                     { ADVANCE_COL; return LT; }
">"                     { ADVANCE_COL; return GT; }
"<="                    { ADVANCE_COL; return LE; }
">="                    { ADVANCE_COL; return GE; }

"("                     { ADVANCE_COL; return LPAREN; }
")"                     { ADVANCE_COL; return RPAREN; }
"{"                     { ADVANCE_COL; return LBRACE; }
"}"                     { ADVANCE_COL; return RBRACE; }
","                     { ADVANCE_COL; return COMMA; }
";"                     { ADVANCE_COL; return SEMICOLON; }

"&&"                    { ADVANCE_COL; return AND; }
"||"                    { ADVANCE_COL; return OR; }
"!"                     { ADVANCE_COL; return NOT; }
"="                     { ADVANCE_COL; return ASSIGN; }

.                       { /* Caractere não reconhecido */
                          char msg[128];
                          snprintf(msg, sizeof msg, "Caractere não reconhecido: %s", yytext);
                          yyerror(msg);       /* incrementa g_parse_errors no parser.y */
                          ADVANCE_COL;        /* consumiu 1 char */
                          return ERROR;   /* força erro de sintaxe no Bison */
                        }

%%

int yywrap(void) { return 1; }
