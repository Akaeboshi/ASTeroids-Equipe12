
%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}

/* Tipos de valores semânticos possíveis. */
%union {
    int intValue;
}

/* Tokens que carregam valor semântico */
%token <intValue> NUM

/* Tokens sem valor semântico */
%token PLUS MINUS TIMES DIVIDE LPAREN RPAREN

/* Regras de precedência e associatividade */
%left PLUS MINUS
%left TIMES DIVIDE

/* Associação de tipos semânticos a não-terminais */
%type <intValue> expr

%%

expr:
      expr PLUS expr    { $$ = $1 + $3; }
    | expr MINUS expr   { $$ = $1 - $3; }
    | expr TIMES expr   { $$ = $1 * $3; }
    | expr DIVIDE expr  { $$ = $1 / $3; }
    | LPAREN expr RPAREN{ $$ = $2; }
    | NUM               { $$ = $1; }
    ;

%%

int main(void) {
    return yyparse();
}

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático: %s\n", s);
}
