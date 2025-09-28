
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
%left EQ NEQ
%left LT LE GT GE
%left PLUS MINUS
%left TIMES DIVIDE

/* Associação de tipos semânticos a não-terminais */
%type <node>  Input Line Expr RelExpr AddExpr MulExpr Primary Num

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
    : RelExpr                             { $$ = $1; }
    ;

RelExpr
    : AddExpr                             { $$ = $1; }
    | RelExpr EQ AddExpr                  { $$ = ast_binary(BIN_EQ, $1, $3); }
    | RelExpr NEQ AddExpr                 { $$ = ast_binary(BIN_NEQ, $1, $3); }
    | RelExpr LT AddExpr                  { $$ = ast_binary(BIN_LT, $1, $3); }
    | RelExpr LE AddExpr                  { $$ = ast_binary(BIN_LE, $1, $3); }
    | RelExpr GT AddExpr                  { $$ = ast_binary(BIN_GT, $1, $3); }
    | RelExpr GE AddExpr                  { $$ = ast_binary(BIN_GE, $1, $3); }
    ;

AddExpr
    : MulExpr                             { $$ = $1; }
    | AddExpr PLUS MulExpr                { $$ = ast_binary(BIN_ADD, $1, $3); }
    | AddExpr MINUS MulExpr               { $$ = ast_binary(BIN_SUB, $1, $3); }
    ;

MulExpr
    : Primary                             { $$ = $1; }
    | MulExpr TIMES Primary               { $$ = ast_binary(BIN_MUL, $1, $3); }
    | MulExpr DIVIDE Primary              { $$ = ast_binary(BIN_DIV, $1, $3); }
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
