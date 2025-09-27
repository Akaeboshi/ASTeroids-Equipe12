%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int yylex(void);
void yyerror(const char *s);
extern int lineno;

// Tabela de símbolos simples
#define MAX_SYMBOLS 100

typedef struct {
    char *name;
    int type; // 0: int, 1: float, 2: bool
    union {
        int intVal;
        double floatVal;
        int boolVal;
    } value;
} Symbol;

Symbol symbolTable[MAX_SYMBOLS];
int symbolCount = 0;

// Funções da tabela de símbolos
Symbol* findSymbol(char *name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return &symbolTable[i];
        }
    }
    return NULL;
}

void addSymbol(char *name, int type) {
    if (findSymbol(name) != NULL) {
        fprintf(stderr, "Erro linha %d: Variável '%s' já declarada\n", lineno, name);
        return;
    }
    if (symbolCount >= MAX_SYMBOLS) {
        fprintf(stderr, "Erro: Tabela de símbolos cheia\n");
        return;
    }
    symbolTable[symbolCount].name = strdup(name);
    symbolTable[symbolCount].type = type;
    // Inicializar com valor padrão
    if (type == 0) symbolTable[symbolCount].value.intVal = 0;
    else if (type == 1) symbolTable[symbolCount].value.floatVal = 0.0;
    else symbolTable[symbolCount].value.boolVal = 0;
    symbolCount++;
}

int checkTypeCompatibility(int varType, double value, int isIntExpr) {
    if (varType == 0) { // int
        if (floor(value) == value && value >= -2147483648 && value <= 2147483647) {
            return 1; // Compatível
        } else {
            fprintf(stderr, "Erro linha %d: Valor não inteiro ou fora do range para variável int\n", lineno);
            return 0;
        }
    } else if (varType == 1) { // float
        return 1; // Sempre compatível (conversão implícita)
    } else { // bool
        if (value == 0.0 || value == 1.0) {
            return 1; // Compatível
        } else {
            fprintf(stderr, "Erro linha %d: Valor não booleano para variável bool (deve ser 0 ou 1)\n", lineno);
            return 0;
        }
    }
}

void updateSymbolValue(char *name, double value) {
    Symbol *sym = findSymbol(name);
    if (sym == NULL) {
        fprintf(stderr, "Erro linha %d: Variável '%s' não declarada\n", lineno, name);
        return;
    }
    
    if (!checkTypeCompatibility(sym->type, value, 0)) {
        return;
    }
    
    if (sym->type == 0) { // int
        sym->value.intVal = (int)value;
    } else if (sym->type == 1) { // float
        sym->value.floatVal = value;
    } else { // bool
        sym->value.boolVal = (int)value;
    }
}

double getSymbolValue(char *name) {
    Symbol *sym = findSymbol(name);
    if (sym == NULL) {
        fprintf(stderr, "Erro linha %d: Variável '%s' não declarada\n", lineno, name);
        return 0.0;
    }
    if (sym->type == 0) { // int
        return (double)sym->value.intVal;
    } else if (sym->type == 1) { // float
        return sym->value.floatVal;
    } else { // bool
        return (double)sym->value.boolVal;
    }
}

void printSymbolTable() {
    printf("\n=== TABELA DE SÍMBOLOS ===\n");
    for (int i = 0; i < symbolCount; i++) {
        if (symbolTable[i].type == 0) {
            printf("  %s: int = %d\n", symbolTable[i].name, symbolTable[i].value.intVal);
        } else if (symbolTable[i].type == 1) {
            printf("  %s: float = %f\n", symbolTable[i].name, symbolTable[i].value.floatVal);
        } else {
            printf("  %s: bool = %s\n", symbolTable[i].name, symbolTable[i].value.boolVal ? "true" : "false");
        }
    }
    printf("==========================\n");
}

// Função para verificar se todos os identificadores usados foram declarados
void checkAllIdentifiersDeclared() {
    // Esta função pode ser expandida para verificar uso de variáveis não declaradas
    printf("Verificação de variáveis concluída.\n");
}
%}

%union {
    int intValue;
    double floatValue;
    char *stringValue;
    int boolValue;
}

%token <intValue> INT_LIT
%token <floatValue> FLOAT_LIT
%token <boolValue> BOOL_LIT
%token <stringValue> IDENTIFIER
%token PLUS MINUS TIMES DIVIDE LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA
%token EQ NEQ LT GT LE GE
%token AND OR NOT ASSIGN
%token INT_KW FLOAT_KW BOOL_KW IF ELSE WHILE FOR RETURN VOID

%left OR
%left AND
%right NOT
%left EQ NEQ
%left LT GT LE GE
%left PLUS MINUS
%left TIMES DIVIDE

%type <floatValue> expression arithmetic_expr term factor
%type <intValue> integer type
%type <stringValue> identifier

%start program

%%

program:
    declarations statements
    ;

declarations:
    /* vazio */
    | declarations variable_decl
    ;

variable_decl:
    type identifier SEMICOLON { 
        int t;
        if ($1 == 0) t = 0; // INT_KW
        else if ($1 == 1) t = 1; // FLOAT_KW
        else t = 2; // BOOL_KW
        addSymbol($2, t);
        printf("Declarada variável: %s (tipo: %d)\n", $2, t);
    }
    ;

type:
    INT_KW { $$ = 0; }
    | FLOAT_KW { $$ = 1; }
    | BOOL_KW { $$ = 2; }
    ;

statements:
    /* vazio */
    | statements statement
    ;

statement:
    expression SEMICOLON { printf("Expressão executada\n"); }
    | assignment SEMICOLON
    ;

assignment:
    identifier ASSIGN expression { 
        updateSymbolValue($1, $3);
        printf("Atribuição: %s = %f\n", $1, $3);
    }
    ;

expression:
    arithmetic_expr { $$ = $1; printf("Expressão aritmética: %f\n", $$); }
    | expression EQ expression { $$ = $1 == $3 ? 1.0 : 0.0; printf("Comparação ==: %f == %f -> %d\n", $1, $3, (int)$$); }
    | expression NEQ expression { $$ = $1 != $3 ? 1.0 : 0.0; printf("Comparação !=: %f != %f -> %d\n", $1, $3, (int)$$); }
    | expression LT expression { $$ = $1 < $3 ? 1.0 : 0.0; printf("Comparação <: %f < %f -> %d\n", $1, $3, (int)$$); }
    | expression GT expression { $$ = $1 > $3 ? 1.0 : 0.0; printf("Comparação >: %f > %f -> %d\n", $1, $3, (int)$$); }
    | expression LE expression { $$ = $1 <= $3 ? 1.0 : 0.0; printf("Comparação <=: %f <= %f -> %d\n", $1, $3, (int)$$); }
    | expression GE expression { $$ = $1 >= $3 ? 1.0 : 0.0; printf("Comparação >=: %f >= %f -> %d\n", $1, $3, (int)$$); }
    | expression AND expression { $$ = ($1 != 0.0 && $3 != 0.0) ? 1.0 : 0.0; printf("Operador AND: %f && %f -> %d\n", $1, $3, (int)$$); }
    | expression OR expression { $$ = ($1 != 0.0 || $3 != 0.0) ? 1.0 : 0.0; printf("Operador OR: %f || %f -> %d\n", $1, $3, (int)$$); }
    | NOT expression { $$ = $2 == 0.0 ? 1.0 : 0.0; printf("Operador NOT: !%f -> %d\n", $2, (int)$$); }
    ;

arithmetic_expr:
    term { $$ = $1; }
    | arithmetic_expr PLUS term { $$ = $1 + $3; printf("Soma: %f + %f = %f\n", $1, $3, $$); }
    | arithmetic_expr MINUS term { $$ = $1 - $3; printf("Subtração: %f - %f = %f\n", $1, $3, $$); }
    ;

term:
    factor { $$ = $1; }
    | term TIMES factor { $$ = $1 * $3; printf("Multiplicação: %f * %f = %f\n", $1, $3, $$); }
    | term DIVIDE factor { 
        if ($3 == 0.0) {
            fprintf(stderr, "Erro linha %d: Divisão por zero\n", lineno);
            $$ = 0.0;
        } else {
            $$ = $1 / $3; 
            printf("Divisão: %f / %f = %f\n", $1, $3, $$);
        }
    }
    ;

factor:
    integer { $$ = (double)$1; printf("Inteiro: %d\n", $1); }
    | FLOAT_LIT { $$ = $1; printf("Float: %f\n", $1); }
    | BOOL_LIT { $$ = $1 ? 1.0 : 0.0; printf("Boolean: %s\n", $1 ? "true" : "false"); }
    | identifier { 
        $$ = getSymbolValue($1);
        printf("Identificador: %s (valor: %f)\n", $1, $$);
      }
    | LPAREN expression RPAREN { $$ = $2; printf("Parênteses: %f\n", $$); }
    ;

integer:
    INT_LIT { $$ = $1; }
    ;

identifier:
    IDENTIFIER { $$ = $1; }
    ;

%%

int main(void) {
    printf("Parser iniciado. Digite expressões ou 'quit' para sair.\n");
    int result = yyparse();
    checkAllIdentifiersDeclared();
    printSymbolTable();
    return result;
}

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático: %s\n", s);
}