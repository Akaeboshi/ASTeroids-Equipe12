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

// Pilha de escopos
typedef struct ScopeStack {
    SymbolTable *current;
    struct ScopeStack *next;
} ScopeStack;

ScopeStack *scope_stack = NULL;

void push_scope(void) {
    ScopeStack *new_scope = (ScopeStack*)xmalloc(sizeof(ScopeStack));
    new_scope->current = st_create();
    new_scope->next = scope_stack;
    scope_stack = new_scope;
    printf("DEBUG: Escopo criado: %p\n", (void*)new_scope->current);
}

void pop_scope(void) {
    if (scope_stack) {
        ScopeStack *temp = scope_stack;
        scope_stack = scope_stack->next;
        printf("DEBUG: Escopo destruído: %p\n", (void*)temp->current);
        st_destroy(temp->current);
        free(temp);
    }
}

SymbolTable* current_scope(void) {
    return scope_stack ? scope_stack->current : NULL;
}

// Busca recursiva em todos os escopos
Node* lookup_variable(const char *name) {
    ScopeStack *current = scope_stack;
    printf("DEBUG: Buscando variável '%s' na pilha de escopos\n", name);
    while (current) {
        Node *value = st_lookup(current->current, name);
        if (value) {
            printf("DEBUG: Variável '%s' encontrada no escopo %p\n", name, (void*)current->current);
            return value;
        }
        current = current->next;
    }
    printf("DEBUG: Variável '%s' NÃO encontrada em nenhum escopo\n", name);
    return NULL;
}

// Insere no escopo atual
bool insert_variable(const char *name, Node *value) {
    SymbolTable *scope = current_scope();
    if (!scope) {
        printf("DEBUG: ERRO - Nenhum escopo atual para inserir '%s'\n", name);
        return false;
    }
    printf("DEBUG: Inserindo variável '%s' no escopo %p\n", name, (void*)scope);
    return st_insert(scope, name, value);
}
%}

%union {
    struct Node* node;
    int intValue;
    double floatValue;
    int boolValue;
    char *str;
}

%token <intValue> INT_LIT
%token <floatValue> FLOAT_LIT
%token <boolValue> BOOL_LIT
%token <str> IDENT
%token <str> STRING_LIT

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

%type <node> Program StmtList Stmt Block IfStmt WhileStmt ForStmt
%type <node> FunctionDef ParamList ArgList
%type <node> Expr OrExpr AndExpr EqExpr RelExpr AddExpr MulExpr Unary Primary
%type <node> Num AssignExpr

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
    | Block                                 { $$ = $1; }
    | IfStmt                                { $$ = $1; }
    | WhileStmt                             { $$ = NULL; }
    | ForStmt                               { $$ = NULL; }
    | FunctionDef                           { $$ = NULL; }
    | RETURN Expr SEMICOLON                 { ast_free($2); $$ = NULL; }
    | SEMICOLON                             { $$ = NULL; }
    | ERROR                                 { yyerrok; $$ = NULL; }  /* consome erro léxico isolado */
    | error SEMICOLON                       { yyerror("recuperado: instrução inválida"); yyerrok; $$ = NULL; }
    ;

Block
    : LBRACE              { push_scope(); }
      StmtList
      RBRACE              { pop_scope(); $$ = $3; }
    ;

IfStmt
  : IF LPAREN Expr RPAREN Stmt %prec IFX   { $$ = ast_if($3, $5, NULL); }
  | IF LPAREN Expr RPAREN Stmt ELSE Stmt   { $$ = ast_if($3, $5, $7);   }
  ;

WhileStmt
    : WHILE LPAREN Expr RPAREN Stmt         { ast_free($3); ast_free($5); $$ = NULL; }
    ;

ForStmt
    : FOR LPAREN Expr SEMICOLON Expr SEMICOLON Expr RPAREN Stmt
                                            { ast_free($3); ast_free($5); ast_free($7); ast_free($9); $$ = NULL; }
    ;

FunctionDef
    : FUNCTION IDENT LPAREN ParamList RPAREN Block
                                            { free($2); ast_free($6); $$ = NULL; }
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
    : AssignExpr                            { $$ = $1; }
    ;

AssignExpr
    : OrExpr
    | IDENT ASSIGN AssignExpr               {
                                                insert_variable($1, ast_copy($3));
                                                $$ = ast_assign($1, $3);
                                                free($1);
                                            }
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
