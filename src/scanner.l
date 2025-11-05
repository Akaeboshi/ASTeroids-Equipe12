%option yylineno
%option noinput nounput
%x COMMENT

%{
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wsign-compare"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"   /* tokens e yylval do Bison (gerado por bison -d) */

void yyerror(const char *s); /* declarada em parser.y */
extern int g_parse_errors;   /* contador do parser.y */

int yycolumn = 1; /* coluna atual - inicializada em 1 no driver */
#define ADVANCE_COLUMN (yycolumn += yyleng) /* macro para avançar a coluna */

/* * Processa um literal de string (yytext)
 * - Remove as aspas do início e do fim
 * - Processa as sequências de escape
 * - Retorna uma string alocada (deve ser liberada depois)
 */
static char* process_string_literal(const char* text, int len) {
    // Aloca espaço suficiente (nunca será maior que o original)
    char *str = (char*)malloc(len);
    if (!str) return NULL; // Falha de alocação

    int str_index = 0;
    size_t text_index = 1; // Pula a aspa inicial
    size_t end = (size_t)len - 1; // Posição da aspa final

    while (text_index < end) { // Para antes da aspa final
        if (text[text_index] == '\\') {
            text_index++; // Pula a barra
            if (text_index >= end) break; // String termina em barra

            switch (text[text_index]) {
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
                    // Se não for um escape reconhecido, mantém a barra e o caractere
                    str[str_index++] = '\\';
                    str[str_index++] = text[text_index];
                    break;
            }
        } else {
            str[str_index++] = text[text_index];
        }
        text_index++;
    }
    str[str_index] = '\0'; // Terminador nulo

    // Realoca para o tamanho exato para economizar memória
    char *final_str = realloc(str, str_index + 1);
    return final_str ? final_str : str; // Retorna o ponteiro (original ou reallocado)
}
%}

%%

"true"                  { yylval.boolValue = 1; ADVANCE_COLUMN; return BOOL_LIT; } /* booleanos */
"false"                 { yylval.boolValue = 0; ADVANCE_COLUMN; return BOOL_LIT; }

"int"                   { ADVANCE_COLUMN; return KW_INT; }
"float"                 { ADVANCE_COLUMN; return KW_FLOAT; }
"bool"                  { ADVANCE_COLUMN; return KW_BOOL; }
"string"                { ADVANCE_COLUMN; return KW_STRING; }

[0-9]+\.[0-9]+([eE][-+]?[0-9]+)?   { yylval.floatValue = strtod(yytext, NULL); ADVANCE_COLUMN; return FLOAT_LIT; } /* float */
[0-9]+              {
                        /* Use strtol(texto, NULL, base=10) */
                        yylval.intValue = (int)strtol(yytext, NULL, 10);
                        ADVANCE_COLUMN;
                        return INT_LIT;
                    } /* inteiro */

\"([^\\\n]|(\\.))*\"   {
                        yylval.str = process_string_literal(yytext, yyleng);
                        ADVANCE_COLUMN;
                        return STRING_LIT;
                       }

"if"                    { ADVANCE_COLUMN; return IF; } /* palavras-chave */
"else"                  { ADVANCE_COLUMN; return ELSE; }
"while"                 { ADVANCE_COLUMN; return WHILE; }
"for"                   { ADVANCE_COLUMN; return FOR; }
"function"              { ADVANCE_COLUMN; return FUNCTION; }
"return"                { ADVANCE_COLUMN; return RETURN; }
"void"                  { ADVANCE_COLUMN; return KW_VOID; }

[a-zA-Z_][a-zA-Z0-9_]*  { yylval.str = xstrdup(yytext); ADVANCE_COLUMN; return IDENT; } /* identificadores */

[\t ]+                  { ADVANCE_COLUMN; } /* ignora tabs/espacos */
\r\n                    { yycolumn = 1; }
\n                      { yycolumn = 1; }
\r                      { yycolumn = 1; }

"//"[^\n]*              { ADVANCE_COLUMN; } /* comentário de linha */
"/*"                    { ADVANCE_COLUMN; BEGIN(COMMENT); } /* comentário multilinha */

<COMMENT>"*/"           { ADVANCE_COLUMN; BEGIN(INITIAL); } /* fim do comentário */
<COMMENT>\r\n           { yycolumn = 1; } /* nova linha dentro do comentário */
<COMMENT>\n             { yycolumn = 1; }
<COMMENT>.              { ADVANCE_COLUMN; } /* qualquer outro caractere dentro do comentário */
<COMMENT><<EOF>>        {
                          fprintf(stderr, "Erro léxico: comentário multilinha não fechado (linha %d)\n", yylineno);
                          exit(1);
                        }

"=="                    { ADVANCE_COLUMN; return EQ; } /* operadores compostos */
"!="                    { ADVANCE_COLUMN; return NEQ; }
"<="                    { ADVANCE_COLUMN; return LE; }
">="                    { ADVANCE_COLUMN; return GE; }

"&&"                    { ADVANCE_COLUMN; return AND; } /* operadores lógicos */
"||"                    { ADVANCE_COLUMN; return OR; }

"<"                     { ADVANCE_COLUMN; return LT; } /* operadores simples */
">"                     { ADVANCE_COLUMN; return GT; }
"+"                     { ADVANCE_COLUMN; return PLUS; }
"-"                     { ADVANCE_COLUMN; return MINUS; }
"*"                     { ADVANCE_COLUMN; return TIMES; }
"/"                     { ADVANCE_COLUMN; return DIVIDE; }
"!"                     { ADVANCE_COLUMN; return NOT; }
"="                     { ADVANCE_COLUMN; return ASSIGN; }

"("                     { ADVANCE_COLUMN; return LPAREN; } /* símbolos */
")"                     { ADVANCE_COLUMN; return RPAREN; }
"{"                     { ADVANCE_COLUMN; return LBRACE; }
"}"                     { ADVANCE_COLUMN; return RBRACE; }
","                     { ADVANCE_COLUMN; return COMMA; }
";"                     { ADVANCE_COLUMN; return SEMICOLON; }


.                       {
                          char msg[160];
                          snprintf(msg, sizeof msg, "caractere não reconhecido '%s'", yytext);
                          yyerror(msg);           /* usa seu handler do Bison (conta erro/linha/coluna) */
                          ADVANCE_COLUMN;
                          return ERROR;           /* avisa o parser! */
                        } /* ---------- Erro léxico (caractere não reconhecido) ---------- */

%%

int yywrap(void) { return 1; }
