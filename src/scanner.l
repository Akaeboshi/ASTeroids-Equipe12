%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"   /* tokens e yylval do Bison */
%}

%option noinput nounput
%x COMMENT

%%

"var"                   { return VAR; }

true                    { yylval.boolValue = 1; return BOOL_LIT; }
false                   { yylval.boolValue = 0; return BOOL_LIT; }

[0-9]+"."[0-9]+         { yylval.floatValue = strtod(yytext, NULL); return FLOAT_LIT; }
[0-9]+                  { yylval.intValue = atoi(yytext); return INT_LIT; }

\"([^\\\n]|(\\.))*\"    {
                          // Processa string com escapes
                          char *str = (char*)malloc(yyleng); // Aloca espaço suficiente
                          int str_index = 0;
                          int text_index = 1; // Pula a aspas inicial
                          
                          while (text_index < yyleng - 1) { // Para antes da aspas final
                            if (yytext[text_index] == '\\') {
                              // Sequência de escape
                              text_index++;
                              switch (yytext[text_index]) {
                                case 'n': str[str_index++] = '\n'; break;
                                case 't': str[str_index++] = '\t'; break;
                                case 'r': str[str_index++] = '\r'; break;
                                case 'b': str[str_index++] = '\b'; break;
                                case 'f': str[str_index++] = '\f'; break;
                                case 'v': str[str_index++] = '\v'; break;
                                case '\\': str[str_index++] = '\\'; break;
                                case '\"': str[str_index++] = '\"'; break;
                                case '\'': str[str_index++] = '\''; break;
                                case '0': str[str_index++] = '\0'; break;
                                default: 
                                  // Se não for um escape reconhecido, mantém o caractere
                                  str[str_index++] = '\\';
                                  str[str_index++] = yytext[text_index];
                                  break;
                              }
                            } else {
                              str[str_index++] = yytext[text_index];
                            }
                            text_index++;
                          }
                          str[str_index] = '\0';
                          
                          // Realoca para o tamanho exato
                          char *final_str = realloc(str, str_index + 1);
                          yylval.str = final_str ? final_str : str;
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
