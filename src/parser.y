
%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}

/* Tipos de valores semânticos possíveis. */
%union {
    int intValue;
    double floatValue;
    int boolValue;
}

/* Tokens com valor semântico */
%token <intValue> INT_LIT
%token <floatValue> FLOAT_LIT
%token <boolValue> BOOL_LIT

/* Tokens sem valor semântico */
%token PLUS MINUS TIMES DIVIDE LPAREN RPAREN    /* Tokens de símbolos fixos */
%token EQ NEQ LT GT LE GE                       /* Tokens dos operadores relacionais */
%token AND OR NOT                               /* Tokens dos operadores lógicos */
%token ASSIGN SEMICOLON                         /* Tokens de atribuição e terminador */

/* Regras de precedência e associatividade */
%left PLUS MINUS
%left TIMES DIVIDE

/* Associação de tipos semânticos a não-terminais */
%type <floatValue> Num Primary expr

/* símbolo inicial */
%start input

%%

input
    : /* vazio */
    | input line
    ;

line
    : expr SEMICOLON                      { printf("Resultado: %f\n", $1); }
    | SEMICOLON                           { /* linha vazia */ }
    ;

Num:
      INT_LIT                             { $$ = (double)$1; }
    | FLOAT_LIT                           { $$ = $1; }
    ;

Primary
    : LPAREN expr RPAREN                  { $$ = $2; }
    | Num
    ;

expr
    : expr PLUS expr                      { $$ = $1 + $3; }
    | expr MINUS expr                     { $$ = $1 - $3; }
    | expr TIMES expr                     { $$ = $1 * $3; }
    | expr DIVIDE expr                    { $$ = $1 / $3; }
    | Primary                             { $$ = $1; }
    ;

%%

int main(void) {
    return yyparse();
}

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático: %s\n", s);
}
