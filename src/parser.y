
%{
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}

/* Tipos de valores semânticos possíveis. */
%union {
    struct Node* node;
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
%type <node>  Input Line Expr Primary Num

/* Símbolo inicial */
%start Input

%%

Input
    : /* vazio */
    | Input Line
    ;

Line
    : Expr SEMICOLON                      { ast_print($1); ast_print_pretty($1); ast_free($1); }
    | SEMICOLON                           { /* linha vazia */ }
    ;

Expr
    : Expr PLUS Expr                      { $$ = ast_binary(BIN_ADD, $1, $3); }
    | Expr MINUS Expr                     { $$ = ast_binary(BIN_SUB, $1, $3); }
    | Expr TIMES Expr                     { $$ = ast_binary(BIN_MUL, $1, $3); }
    | Expr DIVIDE Expr                    { $$ = ast_binary(BIN_DIV, $1, $3); }
    | Primary                             { $$ = $1; }
    ;

Num
    : INT_LIT                             { $$ = ast_int($1); }
    | FLOAT_LIT                           { $$ = ast_float($1); }
    | BOOL_LIT                            { $$ = ast_bool($1); }
    ;

Primary
    : LPAREN Expr RPAREN                  { $$ = $2; }
    | Num
    ;


%%

int main(void) {
    return yyparse();
}

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático: %s\n", s);
}
