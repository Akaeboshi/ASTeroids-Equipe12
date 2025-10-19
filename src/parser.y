%{
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}

/* Tipos de valores semânticos */
%union {
    struct Node* node;
    int intValue;
    double floatValue;
    int boolValue;
    char *str;
}

/* Tokens com valor semântico */
%token <intValue> INT_LIT
%token <floatValue> FLOAT_LIT
%token <boolValue> BOOL_LIT
%token <str> IDENT
%token <str> STRING_LIT   /* novo */

/* Tokens sem valor semântico */
%token LPAREN RPAREN LBRACE RBRACE
%token PLUS MINUS TIMES DIVIDE
%token EQ NEQ LT GT LE GE
%token AND OR NOT
%token ASSIGN
%token COMMA SEMICOLON
%token IF ELSE WHILE FOR FUNCTION RETURN

/* Regras de precedência e associatividade */
%left OR
%left AND
%left EQ NEQ
%left LT LE GT GE
%left PLUS MINUS
%left TIMES DIVIDE
%right NOT
%right UMINUS

%precedence IFX
%precedence ELSE

/* Tipagem dos não-terminais com AST */
%type <node> Program StmtList Stmt Block IfStmt WhileStmt ForStmt
%type <node> FunctionDef ParamList ArgList
%type <node> Expr OrExpr AndExpr EqExpr RelExpr AddExpr MulExpr Unary Primary
%type <node> Num

/* Símbolo inicial */
%start Program

%%

Program
    : StmtList
    ;

StmtList
    : %empty                                { $$ = NULL; }
    | StmtList Stmt                         { $$ = NULL; }
    ;

Stmt
    : Expr SEMICOLON                        { $$ = $1; ast_print($$); ast_print_pretty($$); ast_free($$); $$ = NULL; }
    | Block                                 { $$ = NULL; }
    | IfStmt                                { $$ = $1; }
    | WhileStmt                             { $$ = $1; }
    | ForStmt                               { $$ = $1; }
    | FunctionDef                           { $$ = NULL; }
    | RETURN Expr SEMICOLON                 { ast_free($2); $$ = NULL; }
    | SEMICOLON                             { $$ = NULL; }
    ;

Block
    : LBRACE StmtList RBRACE                { $$ = NULL; }
    ;

IfStmt
    : IF LPAREN Expr RPAREN Stmt            { ast_free($3); $$ = NULL; } %prec IFX
    | IF LPAREN Expr RPAREN Stmt ELSE Stmt  { ast_free($3); $$ = NULL; }
    ;

WhileStmt
    : WHILE LPAREN Expr RPAREN Stmt         { ast_free($3); $$ = NULL; }
    ;

ForStmt
    : FOR LPAREN Expr SEMICOLON Expr SEMICOLON Expr RPAREN Stmt
                                          { ast_free($3); ast_free($5); ast_free($7); $$ = NULL; }
    ;

FunctionDef
    : FUNCTION IDENT LPAREN ParamList RPAREN Block
                                          { free($2); $$ = NULL; }
    ;

ParamList
    : %empty                                { $$ = NULL; }
    | IDENT                                 { free($1); $$ = NULL; }
    | ParamList COMMA IDENT                 { free($3); $$ = NULL; }
    ;

ArgList
    : %empty                                { $$ = NULL; }
    | Expr                                  { $$ = $1; }
    | ArgList COMMA Expr                    { ast_free($3); $$ = $1; }
    ;

Expr
    : OrExpr                                { $$ = $1; }
    ;

OrExpr
    : AndExpr                               { $$ = $1; }
    | OrExpr OR AndExpr                     { $$ = ast_binary(BIN_OR, $1, $3); }
    ;

AndExpr
    : EqExpr                                { $$ = $1; }
    | AndExpr AND EqExpr                    { $$ = ast_binary(BIN_AND, $1, $3); }
    ;

EqExpr
    : RelExpr                               { $$ = $1; }
    | EqExpr EQ RelExpr                     { $$ = ast_binary(BIN_EQ, $1, $3); }
    | EqExpr NEQ RelExpr                    { $$ = ast_binary(BIN_NEQ, $1, $3); }
    ;

RelExpr
    : AddExpr                               { $$ = $1; }
    | RelExpr LT AddExpr                    { $$ = ast_binary(BIN_LT, $1, $3); }
    | RelExpr LE AddExpr                    { $$ = ast_binary(BIN_LE, $1, $3); }
    | RelExpr GT AddExpr                    { $$ = ast_binary(BIN_GT, $1, $3); }
    | RelExpr GE AddExpr                    { $$ = ast_binary(BIN_GE, $1, $3); }
    ;

AddExpr
    : MulExpr                               { $$ = $1; }
    | AddExpr PLUS MulExpr                  { $$ = ast_binary(BIN_ADD, $1, $3); }
    | AddExpr MINUS MulExpr                 { $$ = ast_binary(BIN_SUB, $1, $3); }
    ;

MulExpr
    : Unary                                 { $$ = $1; }
    | MulExpr TIMES Unary                   { $$ = ast_binary(BIN_MUL, $1, $3); }
    | MulExpr DIVIDE Unary                  { $$ = ast_binary(BIN_DIV, $1, $3); }
    ;

Unary
    : Primary                               { $$ = $1; }
    | NOT Unary                             { $$ = ast_unary(UN_NOT, $2); }
    | MINUS Unary  %prec UMINUS             { $$ = ast_unary(UN_NEG, $2); }
    ;

Primary
    : LPAREN Expr RPAREN                    { $$ = $2; }
    | Num                                   { $$ = $1; }
    | IDENT                                 { $$ = ast_ident($1); free($1); }
    | IDENT LPAREN ArgList RPAREN           { free($1); ast_free($3); $$ = NULL; }
    | STRING_LIT                            { $$ = ast_string($1); free($1); } /* novo */
    ;

Num
    : INT_LIT                               { $$ = ast_int($1); }
    | FLOAT_LIT                             { $$ = ast_float($1); }
    | BOOL_LIT                              { $$ = ast_bool($1); }
    ;

%%

int main(void) {
    return yyparse();
}

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático: %s\n", s);
}
