%{
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

Node *g_program_ast = NULL;
int g_parse_errors = 0;

extern int yylineno;
extern int yycolumn;

int yylex(void);

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático (%d:%d): %s\n", yylineno, yycolumn, s);
    g_parse_errors++;
}

/* Buffers temporários (parâmetros de função) */
static Node  **g_params_buf = NULL;
static size_t  g_params_len = 0;
%}

%code requires {
  #include "ast_base.h"
}

%union {
    struct Node* node;
    int intValue;
    double floatValue;
    int boolValue;
    char *str;
    TypeTag typeTag;
}

%token <intValue> INT_LIT
%token <floatValue> FLOAT_LIT
%token <boolValue> BOOL_LIT
%token <str> IDENT
%token <str> STRING_LIT

%token KW_INT
%token KW_FLOAT
%token KW_BOOL
%token KW_STRING
%token KW_VOID

%token LPAREN RPAREN LBRACE RBRACE
%token PLUS MINUS TIMES DIVIDE
%token EQ NEQ LT GT LE GE
%token AND OR NOT
%token ASSIGN
%token COMMA SEMICOLON
%token IF ELSE WHILE FOR FUNCTION RETURN
%token ERROR

%left OR
%left AND
%left EQ NEQ
%left LT LE GT GE
%left PLUS MINUS
%left TIMES DIVIDE
%right NOT
%right UMINUS
%right ASSIGN

%precedence IFX
%precedence ELSE

/* Habilita mensagens de erro detalhadas */
%define parse.error verbose

%type <typeTag> TypeTag
%type <node>    Program StmtList Stmt Block IfStmt WhileStmt ForStmt
%type <node>    FunctionDef ParamList Param ArgList
%type <node>    Expr OrExpr AndExpr EqExpr RelExpr AddExpr MulExpr Unary Primary
%type <node>    Num AssignExpr
%type <node>    Decl

%start Program

%%

Program
    : StmtList                              { g_program_ast = $1; $$ = $1; }
    ;

StmtList
    : %empty                                { $$ = ast_block(); }
    | StmtList Stmt                         { if ($2) ast_block_add_stmt($1, $2); $$ = $1; }
    ;

Stmt
    : Expr SEMICOLON                        { $$ = ast_expr($1); }
    | Decl                                  { $$ = $1; }
    | Block                                 { $$ = $1; }
    | IfStmt                                { $$ = $1; }
    | WhileStmt                             { $$ = $1; }
    | ForStmt                               { $$ = $1; }
    | FunctionDef                           { $$ = $1; }
    | RETURN SEMICOLON                      { $$ = ast_return(NULL); }
    | RETURN Expr SEMICOLON                 { $$ = ast_return($2); }
    | SEMICOLON                             { $$ = NULL; }
    | ERROR                                 { yyerrok; $$ = NULL; }  /* consome erro léxico isolado */
    | error SEMICOLON                       { yyerror("recuperado: instrução inválida"); yyerrok; $$ = NULL; }
    ;

Block
    : LBRACE StmtList RBRACE                { $$ = $2; }
    ;

TypeTag
  : KW_INT                                  { $$ = TY_INT; }
  | KW_FLOAT                                { $$ = TY_FLOAT; }
  | KW_BOOL                                 { $$ = TY_BOOL; }
  | KW_STRING                               { $$ = TY_STRING; }
  | KW_VOID                                 { $$ = TY_VOID; }
  ;

Decl
  : TypeTag IDENT ASSIGN Expr SEMICOLON     { $$ = ast_decl($1, $2, $4); free($2); }
  | TypeTag IDENT SEMICOLON                 { $$ = ast_decl($1, $2, NULL); free($2); }
  ;

IfStmt
  : IF LPAREN Expr RPAREN Stmt %prec IFX    { $$ = ast_if($3, $5, NULL); }
  | IF LPAREN Expr RPAREN Stmt ELSE Stmt    { $$ = ast_if($3, $5, $7); }
  ;

WhileStmt
    : WHILE LPAREN Expr RPAREN Stmt         { $$ = ast_while($3, $5); }
    ;

ForStmt
    : FOR LPAREN Expr SEMICOLON Expr SEMICOLON Expr RPAREN Stmt
                                            { $$ = ast_for($3, $5, $7, $9); }
    ;

FunctionDef
  : TypeTag IDENT LPAREN ParamList RPAREN   {
                                              Node *plist = $4;
                                              g_params_len = plist->u.as_block.count;
                                              if (g_params_len > 0) {
                                                g_params_buf = (Node**)xmalloc(sizeof(Node*) * g_params_len);
                                                for (size_t i = 0; i < g_params_len; i++) {
                                                  g_params_buf[i] = plist->u.as_block.stmts[i];
                                                }
                                              } else { g_params_buf = NULL; }
                                              free(plist->u.as_block.stmts); free(plist);
                                            }
                                            Block
                                            {
                                              $$ = ast_function($1, $2, g_params_buf, g_params_len, $7);
                                              g_params_buf = NULL; g_params_len = 0;
                                            }
  ;

Param
    : TypeTag IDENT                         { $$ = ast_decl($1, $2, NULL); free($2); }
    ;

ParamList
    : %empty                                { $$ = ast_block(); }
    | Param                                 { $$ = ast_block(); ast_block_add_stmt($$, $1); }
    | ParamList COMMA Param                 { ast_block_add_stmt($1, $3); $$ = $1; }
    ;

ArgList
    : %empty                                { $$ = NULL; }
    | Expr                                  { $$ = $1; }
    | ArgList COMMA Expr                    { ast_free($3); $$ = $1; }
    ;

Expr
    : AssignExpr                            { $$ = $1; }
    ;

AssignExpr
    : OrExpr
    | IDENT ASSIGN AssignExpr               { $$ = ast_assign($1, $3); free($1); }
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
    | IDENT LPAREN ArgList RPAREN           { free($1); $$ = NULL; }
    | STRING_LIT                            { $$ = ast_string($1); free($1); }
    ;

Num
    : INT_LIT                               { $$ = ast_int($1); }
    | FLOAT_LIT                             { $$ = ast_float($1); }
    | BOOL_LIT                              { $$ = ast_bool($1); }
    ;

%%
