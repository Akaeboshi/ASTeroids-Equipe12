
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
    char *str;
    int boolValue;
}

/* Tokens que carregam valor semântico */
%token <intValue> INT_LIT
%token <floatValue> FLOAT_LIT
%token <boolValue> BOOL_LIT
%token <str> IDENT

/* Tokens sem valor semântico */
%token PLUS MINUS TIMES DIVIDE LPAREN RPAREN 

/* Tokens dos operadores relacionais */
%token EQ NEQ LT GT LE GE

/* Tokens dos operadores lógicos e de atribuição */
%token AND OR NOT ASSIGN
%token LBRACE RBRACE SEMICOLON COMMA

/*Condicionais */
%token IF ELSE  WHILE FOR
%token FUNCTION RETURN
%token IDENT

/* Regras de precedência e associatividade */
%left OR
%left AND
%left LT GT LE GE
%left PLUS MINUS
%left TIMES DIVIDE

/* Associação de tipos semânticos a não-terminais */
%type <floatValue> expr arithmetic_expr
%type <floatValue> Num Primary 
%type <boolValue> bool_expr 

/* símbolo inicial */
%start input

%%

input:
      /* vazio */
    | input expr
    ;

stmt_list:
    stmt_list stmt 
    | /* vazio */
    ;


/* Statements  */
stmt:
        expr SEMICOLON
    | block
    | if_stmt
    | while_stmt
    | for_stmt
    | RETURN expr SEMICOLON
    | SEMICOLON
    ;

/* bloco */
block:
     LBRACE stmt_list RBRACE
    ;

/* if/else */
if_stmt:
     IF LPAREN expr RPAREN stmt
     | IF LPAREN expr RPAREN stmt ELSE stmt
    ;

/* while */
while_stmt:
     WHILE LPAREN expr RPAREN stmt
    ;

/* for */
     FOR LPAREN expr SEMICOLON expr SEMICOLON expr RPAREN stmt  
    ;

/* funçoes */
function_def:
     FUNCTION IDENT LPAREN param_list RPAREN block
    ;

param_list:
    /* vazio */
    | IDENT
    | param_list COMMA IDENT
    ;

func_call:
    IDENT LPAREN arg_list RPAREN
    ;

arg_list:
    /* vazio */
    | expr
    | arg_list COMMA expr   
    ;

expr:
    arithmetic_expr
    | bool_expr
    | IDENT ASSIGN expr
    | IDENT LPAREN arg_list RPAREN
    ;

arithmetic_expr:
    arithmetic_expr PLUS arithmetic_expr    { $$ = $1 + $3; }
    | arithmetic_expr MINUS arithmetic_expr { $$ = $1 - $3; }
    | arithmetic_expr TIMES arithmetic_expr { $$ = $1 * $3; }
    | arithmetic_expr DIVIDE arithmetic_expr { $$ = $1 / $3; }
    | primary                              { $$ = $1; }
    ;

bool_expr:
    expr EQ expr    { $$ = $1 == $3; }
    | expr NEQ expr { $$ = $1 != $3; }
    | expr LT expr  { $$ = $1 < $3; }
    | expr GT expr  { $$ = $1 > $3; }
    | expr LE expr  { $$ = $1 <= $3; }
    | expr GE expr  { $$ = $1 >= $3; }
    | BOOL_LIT      { $$ = $1; }
    ;



/* expressões */
Num:
      INT_LIT                             { $$ = (double)$1; }
    | FLOAT_LIT                           { $$ = $1; }
    ;

Primary:
      LPAREN expr RPAREN                  { $$ = $2; }
    | Num
    ;

expr:
      expr PLUS expr                      { $$ = $1 + $3; }
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
