#ifndef AST_BASE_H
#define AST_BASE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  TY_INVALID = 0,
  TY_INT,
  TY_FLOAT,
  TY_BOOL,
  TY_STRING,
  TY_VOID,
} TypeTag;

typedef enum {
  ND_INT,       // literal inteiro
  ND_FLOAT,     // literal float
  ND_BOOL,      // literal booleano (true/false)
  ND_IDENT,     // identificador (nome de variável)
  ND_STRING,    // literal string
  ND_UNARY,     // expressão unária (-, !)
  ND_BINARY,    // expressão binária (+, -, *, /)
  ND_BLOCK,     // bloco de código ({ stmt* })
  ND_ASSIGN,    // atribuição (identificador = expr)
  ND_EXPR,      // expressão genérica
  ND_IF,        // instrução if
  ND_DECL,      // declaração de variável
  ND_WHILE,     // instrução while
  ND_FOR,       // instrução for
  ND_FUNCTION,  // declaração de função
  ND_RETURN,    // instrução return
  ND_CALL       // chamada de função
} NodeKind;

typedef enum {
  /* Aritméticos */
  BIN_ADD,  // +
  BIN_SUB,  // -
  BIN_MUL,  // *
  BIN_DIV,  // /

  /* Relacionais */
  BIN_EQ,   // ==
  BIN_NEQ,  // !=
  BIN_LT,   // <
  BIN_LE,  // <=
  BIN_GT,   // >
  BIN_GE,  // >=

  /* Lógicos */
  BIN_AND,  // &&
  BIN_OR    // ||
} BinOp;

typedef enum {
  UN_NEG, // -x
  UN_NOT  // !x
} UnOp;

typedef struct Node Node;

struct Node {
  NodeKind kind;

  union {
    struct { long value; } as_int;
    struct { double value ;} as_float;
    struct { bool value; } as_bool;
    struct { char *name; } as_ident;
    struct { char *value; } as_string;
    struct { UnOp op; Node *expr; } as_unary;
    struct { BinOp op; Node *left; Node *right; } as_binary;
    struct { Node **stmts; size_t count; size_t capacity; } as_block;
    struct { char *name; Node *value; } as_assign;
    struct { Node *expr; } as_expr;
    struct { Node *cond; Node *then_branch; Node *else_branch; } as_if;
    struct { TypeTag type; char *name; Node *init; } as_decl;
    struct { Node *cond; Node *body; } as_while;
    struct { Node *init; Node *cond; Node *step; Node *body; } as_for;
    struct { TypeTag ret_type; char *name; struct Node **params; size_t param_count; struct Node *body; } as_function;
    struct { Node *expr; } as_return;
    struct { char *name; Node *args; } as_call;
  } u;
};

/* Helper Functions */
void *xmalloc(size_t size);
char *xstrdup(const char *s);
struct Node *new_node(NodeKind kind);
Node *ast_copy(Node *node);

#endif /* AST_BASE_H */
