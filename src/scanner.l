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

/* coluna atual (mantida manualmente) */
int yycolumn = 1;

/* Função simples para transformar sequências de escape em caracteres reais.
   Recebe uma string *sem* aspas (ex: entrada: hello\nworld -> processa \n -> newline).
   Retorna malloc()'ed string (caller must free). */
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

/* ---------- Literais e palavras-chave ---------- */

/* booleanos */
"true"                  { yylval.boolValue = 1; yycolumn += yyleng; return BOOL_LIT; }
"false"                 { yylval.boolValue = 0; yycolumn += yyleng; return BOOL_LIT; }

/* float (colocado antes de int) - aceita ex: 3.14 e 1.2e-3 */
[0-9]+\.[0-9]+([eE][-+]?[0-9]+)?   { yylval.floatValue = strtod(yytext, NULL); yycolumn += yyleng; return FLOAT_LIT; }

/* int */
[0-9]+                  { yylval.intValue = atoi(yytext); yycolumn += yyleng; return INT_LIT; }

/* string com escapes - aceita \" \\n \\t etc.
   yytext inclui as aspas, por isso removemos a primeira e última char. */
\"([^\\\n]|(\\.))*\"    {
                          size_t raw_len = (yyleng >= 2) ? yyleng - 2 : 0;
                          char *raw = malloc(raw_len + 1);
                          if (!raw) { fprintf(stderr, "Erro de memória\n"); exit(1); }
                          if (raw_len) strncpy(raw, yytext + 1, raw_len);
                          raw[raw_len] = '\0';
                          char *un = unescape_string(raw);
                          free(raw);
                          yylval.str = un;   /* quem consumir deve free() */
                          yycolumn += yyleng;
                          return STRING_LIT;
                        }

/* ---------- Palavras-chave (mantidas explicitamente) ---------- */
"if"                    { yycolumn += yyleng; return IF; }
"else"                  { yycolumn += yyleng; return ELSE; }
"while"                 { yycolumn += yyleng; return WHILE; }
"for"                   { yycolumn += yyleng; return FOR; }
"function"              { yycolumn += yyleng; return FUNCTION; }
"return"                { yycolumn += yyleng; return RETURN; }

/* ---------- Identificador (fallback para palavras-chave) ---------- */
/* Note: regras de palavras-chave vêm antes, então identificadores longos não são 'quebrados'. */
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.str = strdup(yytext); yycolumn += yyleng; return IDENT; }

/* ---------- Espaços e quebras de linha (atualizam coluna/linha) ---------- */
[\t ]+                  { yycolumn += yyleng; /* ignora tabs/espacos */ }
\r\n                    { yylineno++; yycolumn = 1; }
\n                      { yylineno++; yycolumn = 1; }
\r                      { yylineno++; yycolumn = 1; }

/* ---------- Comentários ---------- */
/* comentário linha única (consume até newline, coluna avança pelo comprimento) */
"//"[^\n]*              { yycolumn += yyleng; /* ignora */ }

/* comentário multilinha - entra em estado COMMENT */
"/*"                    { yycolumn += yyleng; BEGIN(COMMENT); }

/* dentro de comentário multilinha: */
<COMMENT>"*/"           { yycolumn += yyleng; BEGIN(INITIAL); }
<COMMENT>\r\n           { yylineno++; yycolumn = 1; }
<COMMENT>\n             { yylineno++; yycolumn = 1; }
<COMMENT>.              { yycolumn += yyleng; /* qualquer outro caractere dentro do comentário */ }
<COMMENT><<EOF>>        {
                          fprintf(stderr, "Erro léxico: comentário multilinha não fechado (linha %d)\n", yylineno);
                          exit(1);
                        }
<COMMENT>.              { ADVANCE_COL; }

/* ---------- Operadores compostos/simbolos (mais específicos antes) ---------- */
"=="                    { yycolumn += yyleng; return EQ; }
"!="                    { yycolumn += yyleng; return NEQ; }
"<="                    { yycolumn += yyleng; return LE; }
">="                    { yycolumn += yyleng; return GE; }

"&&"                    { yycolumn += yyleng; return AND; }
"||"                    { yycolumn += yyleng; return OR; }

/* operadores simples */
"<"                     { yycolumn += yyleng; return LT; }
">"                     { yycolumn += yyleng; return GT; }
"+"                     { yycolumn += yyleng; return PLUS; }
"-"                     { yycolumn += yyleng; return MINUS; }
"*"                     { yycolumn += yyleng; return TIMES; }
"/"                     { yycolumn += yyleng; return DIVIDE; }
"!"                     { yycolumn += yyleng; return NOT; }
"="                     { yycolumn += yyleng; return ASSIGN; }

/* símbolos */
"("                     { yycolumn += yyleng; return LPAREN; }
")"                     { yycolumn += yyleng; return RPAREN; }
"{"                     { yycolumn += yyleng; return LBRACE; }
"}"                     { yycolumn += yyleng; return RBRACE; }
","                     { yycolumn += yyleng; return COMMA; }
";"                     { yycolumn += yyleng; return SEMICOLON; }

/* ---------- Erro léxico (caracter não reconhecido) ---------- */
.                       {
                          fprintf(stderr, "Erro léxico na linha %d, coluna %d: caractere não reconhecido '%s'\n",
                                  yylineno, yycolumn, yytext);
                          yycolumn += yyleng;
                        }

%%

int yywrap(void) { return 1; }
