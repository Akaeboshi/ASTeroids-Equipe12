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
%token VAR

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

%precedence IFX
%precedence ELSE

/* Habilita mensagens de erro detalhadas */
%define parse.error verbose

%type <node> Program StmtList Stmt Block IfStmt WhileStmt ForStmt
%type <node> FunctionDef ParamList ArgList
%type <node> Expr OrExpr AndExpr EqExpr RelExpr AddExpr MulExpr Unary Primary
%type <node> Num AssignStmt VarDecl

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
    : Expr SEMICOLON                        {
        ast_print($1);
        ast_print_pretty($1);
        $$ = NULL;
    }
    | AssignStmt                            { $$ = NULL; }
    | VarDecl                               { $$ = NULL; }
    | Block                                 { $$ = NULL; }
    | IfStmt                                { $$ = $1; }
    | WhileStmt                             { $$ = NULL; }
    | ForStmt                               { $$ = NULL; }
    | FunctionDef                           { $$ = NULL; }
    | RETURN Expr SEMICOLON                 { ast_free($2); $$ = NULL; }
    | SEMICOLON                             { $$ = NULL; }
    | ERROR { yyerrok; $$ = NULL; }  /* consome erro léxico isolado */
    | error SEMICOLON { yyerror("recuperado: instrução inválida"); yyerrok; $$ = NULL; }
    ;

VarDecl
    : VAR IDENT SEMICOLON                   {
        // Declara variável com valor padrão (0)
        Node *default_value = ast_int(0);
        if (!insert_variable($2, default_value)) {
            yyerror("Variável já declarada no escopo atual");
            free($2);
            ast_free(default_value);
            YYERROR;
        }
        printf("Declarada variável: %s = ", $2);
        ast_print(default_value);
        printf("\n");
        free($2);
        $$ = NULL;
    }
    | VAR IDENT ASSIGN Expr SEMICOLON       {
        // Declara e atribui
        if (!insert_variable($2, ast_copy($4))) {
            yyerror("Variável já declarada no escopo atual");
            free($2);
            ast_free($4);
            YYERROR;
        }
        printf("Declarada e atribuída: %s = ", $2);
        ast_print($4);
        printf("\n");
        free($2);
        $$ = NULL;
    }
    ;

AssignStmt
    : IDENT ASSIGN Expr SEMICOLON           {
        // Verifica se a variável existe em algum escopo
        Node *existing = lookup_variable($1);
        if (!existing) {
            yyerror("Variável não declarada");
            free($1);
            ast_free($3);
            YYERROR;
        }

        // Insere no escopo atual (sobrescreve)
        if (!insert_variable($1, ast_copy($3))) {
            yyerror("Erro ao atribuir variável");
            free($1);
            ast_free($3);
            YYERROR;
        }
        printf("Atribuído: %s = ", $1);
        ast_print($3);
        printf("\n");
        free($1);
        $$ = NULL;
    }
    ;

VarDecl
    : VAR IDENT SEMICOLON                   {
        // Declara variável com valor padrão (0)
        Node *default_value = ast_int(0);
        if (!insert_variable($2, default_value)) {
            yyerror("Variável já declarada no escopo atual");
            free($2);
            ast_free(default_value);
            YYERROR;
        }
        printf("Declarada variável: %s = ", $2);
        ast_print(default_value);
        printf("\n");
        free($2);
        $$ = NULL;
    }
    | VAR IDENT ASSIGN Expr SEMICOLON       {
        // Declara e atribui
        if (!insert_variable($2, ast_copy($4))) {
            yyerror("Variável já declarada no escopo atual");
            free($2);
            ast_free($4);
            YYERROR;
        }
        printf("Declarada e atribuída: %s = ", $2);
        ast_print($4);
        printf("\n");
        free($2);
        $$ = NULL;
    }
    ;

AssignStmt
    : IDENT ASSIGN Expr SEMICOLON           {
        // Verifica se a variável existe em algum escopo
        Node *existing = lookup_variable($1);
        if (!existing) {
            yyerror("Variável não declarada");
            free($1);
            ast_free($3);
            YYERROR;
        }

        // Insere no escopo atual (sobrescreve)
        if (!insert_variable($1, ast_copy($3))) {
            yyerror("Erro ao atribuir variável");
            free($1);
            ast_free($3);
            YYERROR;
        }
        printf("Atribuído: %s = ", $1);
        ast_print($3);
        printf("\n");
        free($1);
        $$ = NULL;
    }
    ;

Block
    : LBRACE { push_scope(); } StmtList { pop_scope(); } RBRACE
        { $$ = NULL; }
    ;

IfStmt
    : IF LPAREN Expr RPAREN Stmt            {
        ast_free($3);
        ast_free($5);
        $$ = NULL;
    }
    | IF LPAREN Expr RPAREN Stmt ELSE Stmt  {
        ast_free($3);
        ast_free($5);
        ast_free($7);
        $$ = NULL;
    }
    ;

WhileStmt
    : WHILE LPAREN Expr RPAREN Stmt         {
        ast_free($3);
        ast_free($5);
        $$ = NULL;
    }
    ;

ForStmt
    : FOR LPAREN Expr SEMICOLON Expr SEMICOLON Expr RPAREN Stmt
        {
            ast_free($3);
            ast_free($5);
            ast_free($7);
            ast_free($9);
            $$ = NULL;
        }
    ;

FunctionDef
    : FUNCTION IDENT LPAREN ParamList RPAREN Block
        {
            free($2);
            $$ = NULL;
        }
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
    | IDENT                                 {
        // Busca o valor em todos os escopos
        Node *value = lookup_variable($1);
        if (value == NULL) {
            yyerror("Variável não definida");
            free($1);
            YYERROR;
        }
        $$ = ast_copy(value);
        free($1);
    }
    | IDENT LPAREN ArgList RPAREN           { free($1); $$ = NULL; }
    | STRING_LIT                            { $$ = ast_string($1); free($1); }
    ;

Num
    : INT_LIT                               { $$ = ast_int($1); }
    | FLOAT_LIT                             { $$ = ast_float($1); }
    | BOOL_LIT                              { $$ = ast_bool($1); }
    ;

%%

int main(void) {
    // Inicializa com escopo global
    push_scope();

    int result = yyparse();

    // Limpa todos os escopos
    while (scope_stack) {
        pop_scope();
    }
    return result;
}

void yyerror(const char *s) {
    fprintf(stderr, "Erro: %s\n", s);
}
