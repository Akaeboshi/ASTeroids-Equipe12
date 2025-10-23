%option yylineno
%option noinput nounput
%x COMMENT

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"   /* tokens e yylval do Bison (gerado por bison -d) */

void yyerror(const char *s); /* declarada em parser.y */
extern int g_parse_errors;   /* contador do parser.y */

int yycolumn = 1; /* coluna atual - inicializada em 1 no driver */
#define ADVANCE_COLUMN (yycolumn += yyleng) /* macro para avançar a coluna */

/* Função simples para transformar sequências de escape em caracteres reais */
static char *unescape_string(const char *src) {
    size_t sl = strlen(src);
    char *out = malloc(sl + 1);
    if (!out) return NULL;
    size_t i = 0, j = 0;
    while (src[i]) {
        if (src[i] == '\\') {
            i++;
            if (!src[i]) break;
            switch (src[i]) {
                case 'n': out[j++] = '\n'; break;
                case 't': out[j++] = '\t'; break;
                case 'r': out[j++] = '\r'; break;
                case '\\': out[j++] = '\\'; break;
                case '"': out[j++] = '"'; break;
                case '\'': out[j++] = '\''; break;
                default: out[j++] = src[i]; break;
            }
            i++;
        } else {
            out[j++] = src[i++];
        }
    }
    out[j] = '\0';
    return out;
}
%}

%%

"true"                  { yylval.boolValue = 1; ADVANCE_COLUMN; return BOOL_LIT; } /* booleanos */
"false"                 { yylval.boolValue = 0; ADVANCE_COLUMN; return BOOL_LIT; }

[0-9]+\.[0-9]+([eE][-+]?[0-9]+)?   { yylval.floatValue = strtod(yytext, NULL); ADVANCE_COLUMN; return FLOAT_LIT; } /* float */
[0-9]+                  { yylval.intValue = atoi(yytext); ADVANCE_COLUMN; return INT_LIT; } /* inteiro */

\"([^\\\n]|(\\.))*\"    { /* string com escapes - aceita \" \\n \\t etc. */
                          size_t raw_len = (yyleng >= 2) ? yyleng - 2 : 0;
                          char *raw = malloc(raw_len + 1);
                          if (!raw) { fprintf(stderr, "Erro de memória\n"); exit(1); }
                          if (raw_len) strncpy(raw, yytext + 1, raw_len);
                          raw[raw_len] = '\0';
                          char *un = unescape_string(raw);
                          free(raw);
                          yylval.str = un;   /* quem consumir deve free() */
                          ADVANCE_COLUMN;
                          return STRING_LIT;
                        }

"if"                    { ADVANCE_COLUMN; return IF; } /* palavras-chave */
"else"                  { ADVANCE_COLUMN; return ELSE; }
"while"                 { ADVANCE_COLUMN; return WHILE; }
"for"                   { ADVANCE_COLUMN; return FOR; }
"function"              { ADVANCE_COLUMN; return FUNCTION; }
"return"                { ADVANCE_COLUMN; return RETURN; }

[a-zA-Z_][a-zA-Z0-9_]*  { yylval.str = strdup(yytext); ADVANCE_COLUMN; return IDENT; } /* identificadores */

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
