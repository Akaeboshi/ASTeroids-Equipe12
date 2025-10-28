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
    new_scope->current->parent = scope_stack ? scope_stack->current : NULL;
    new_scope->next = scope_stack;
    scope_stack = new_scope;
    printf("DEBUG: Escopo criado: current=%p parent=%p\n",
        (void*)new_scope->current, (void*)new_scope->current->parent);
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
bool insert_variable(const char *name, TypeTag type, Node *value) {
    SymbolTable *scope = current_scope();
    if (!scope) {
        printf("DEBUG: ERRO - Nenhum escopo atual para inserir '%s'\n", name);
        return false;
    }
    printf("DEBUG: Inserindo variável '%s' no escopo %p\n", name, (void*)scope);
    return st_insert(scope, name, type, value);
}

// Deduz o tipo produzido por uma expressão AST
static TypeTag infer_type(Node *n) {
    switch (n->kind) {
      case ND_INT:    return TY_INT;
      case ND_FLOAT:  return TY_FLOAT;
      case ND_BOOL:   return TY_BOOL;
      case ND_STRING: return TY_STRING;

      case ND_IDENT: {
        bool ok = false;
        TypeTag t = st_lookup_type_recursive(current_scope(), n->u.as_ident.name, &ok);
        if (!ok) {
            fprintf(stderr,
                    "Erro semântico: identificador '%s' não declarado (linha %d)\n",
                    n->u.as_ident.name, yylineno);
            return TY_INVALID;
        }
        return t;
      }

      case ND_UNARY:{
          TypeTag T = infer_type(n->u.as_unary.expr);
          if (T == TY_INVALID) return TY_INVALID;
          if (n->u.as_unary.op == UN_NEG) return (T == TY_INT || T == TY_FLOAT) ? T : TY_INVALID;
          if (n->u.as_unary.op == UN_NOT) return TY_BOOL;

          return T;
      }

      case ND_BINARY: {
          TypeTag L = infer_type(n->u.as_binary.left);
          TypeTag R = infer_type(n->u.as_binary.right);

          if (L == TY_INVALID || R == TY_INVALID) return TY_INVALID;

          switch (n->u.as_binary.op) {
              // Aritméticos -> int/float (promoção simples)
              case BIN_ADD: case BIN_SUB: case BIN_MUL:
                  if (L == TY_BOOL || L == TY_STRING || R == TY_BOOL || R == TY_STRING)
                      return TY_INVALID;
                  if (L == TY_FLOAT || R == TY_FLOAT)
                      return TY_FLOAT;
                  return TY_INT;

              case BIN_DIV:
                  if (L == TY_BOOL || L == TY_STRING || R == TY_BOOL || R == TY_STRING)
                      return TY_INVALID;
                  return TY_FLOAT;

              // Comparações/igualdade -> bool
              case BIN_LT: case BIN_LE: case BIN_GT: case BIN_GE:
              case BIN_EQ: case BIN_NEQ:
                  if (L == TY_INVALID || R == TY_INVALID) return TY_INVALID;
                  if (L != R) return TY_INVALID;
                  return TY_BOOL;

              // Lógicos -> bool
              case BIN_AND: case BIN_OR:
                  return (L == TY_BOOL && R == TY_BOOL) ? TY_BOOL : TY_INVALID;

          }
          return TY_INVALID;
      }

      case ND_ASSIGN:
          return infer_type(n->u.as_assign.value);

      default:
        return TY_INVALID;
    }
}

static Node *default_value_for(TypeTag t) {
    switch (t) {
        case TY_INT:    return ast_int(0);
        case TY_FLOAT:  return ast_float(0.0);
        case TY_BOOL:   return ast_bool(false);
        case TY_STRING: return ast_string("");
        case TY_INVALID: default: return ast_int(0);
    }
}
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
%type <node>    FunctionDef ParamList ArgList
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
    | WhileStmt                             { $$ = NULL; }
    | ForStmt                               { $$ = NULL; }
    | FunctionDef                           { $$ = NULL; }
    | RETURN Expr SEMICOLON                 { ast_free($2); $$ = NULL; }
    | SEMICOLON                             { $$ = NULL; }
    | ERROR                                 { yyerrok; $$ = NULL; }  /* consome erro léxico isolado */
    | error SEMICOLON                       { yyerror("recuperado: instrução inválida"); yyerrok; $$ = NULL; }
    ;

Block
    : LBRACE                                { push_scope(); }
      StmtList
      RBRACE                                { pop_scope(); $$ = $3; }
    ;

TypeTag
  : KW_INT                                  { $$ = TY_INT; }
  | KW_FLOAT                                { $$ = TY_FLOAT; }
  | KW_BOOL                                 { $$ = TY_BOOL; }
  | KW_STRING                               { $$ = TY_STRING; }
  ;

Decl
  : TypeTag IDENT ASSIGN Expr SEMICOLON     {
                                                SymbolTable *sc = current_scope();
                                                if (st_lookup(sc, $2)) {
                                                    yyerror("Erro semântico: variável já declarada neste escopo");
                                                    ast_free($4); free($2); YYERROR;
                                                }

                                                TypeTag inferred_type = infer_type($4);
                                                if (inferred_type != $1) {
                                                    yyerror("Erro semântico: tipo incompatível na inicialização");
                                                    ast_free($4); free($2); YYERROR;
                                                }

                                                $$ = ast_decl($1, $2, $4);
                                                insert_variable($2, $1, ast_copy($4));
                                                free($2);
                                            }
  | TypeTag IDENT SEMICOLON                 {
                                                SymbolTable *sc = current_scope();
                                                if (st_lookup(sc, $2)) {
                                                    yyerror("Erro semântico: variável já declarada neste escopo");
                                                    free($2); YYERROR;
                                                }

                                                $$ = ast_decl($1, $2, NULL);
                                                insert_variable($2, $1, default_value_for($1));
                                                free($2);
                                            }
  ;

IfStmt
  : IF LPAREN Expr RPAREN Stmt %prec IFX    {
                                                if (infer_type($3) != TY_BOOL) {
                                                    yyerror("Erro semântico: condição do if deve ser bool");
                                                    ast_free($3); ast_free($5); YYERROR;
                                                }
                                                $$ = ast_if($3, $5, NULL);
                                            }
  | IF LPAREN Expr RPAREN Stmt ELSE Stmt    {
                                                if (infer_type($3) != TY_BOOL) {
                                                    yyerror("Erro semântico: condição do if deve ser bool");
                                                    ast_free($3); ast_free($5); ast_free($7); YYERROR;
                                                }
                                                $$ = ast_if($3, $5, $7);
                                            }
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
                                                bool found = false;
                                                TypeTag var_type = st_lookup_type_recursive(current_scope(), $1, &found);
                                                if (!found) {
                                                    yyerror("Erro semântico: variável não declarada");
                                                    ast_free($3); free($1); YYERROR;
                                                }

                                                TypeTag inferred_type = infer_type($3);
                                                if (inferred_type != var_type) {
                                                    yyerror("Erro semântico: tipos incompatíveis na atribuição");
                                                    ast_free($3); free($1); YYERROR;
                                                }

                                                Node *right_expr = ($3 && $3->kind == ND_ASSIGN) ? $3->u.as_assign.value : $3;
                                                if (!st_update_recursive(current_scope(), $1, ast_copy(right_expr))) {
                                                    yyerror("Erro semântico: falha ao atualizar variável");
                                                    ast_free($3); free($1); YYERROR;
                                                }

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
