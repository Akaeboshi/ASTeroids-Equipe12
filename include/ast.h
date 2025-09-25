#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  ND_INT,       // literal inteiro
  ND_FLOAT,     // literal float
  ND_BOOL,      // literal booleano (true/false)
  ND_BINARY     // expressão binária (+, -, *, /)
} NodeKind;

typedef enum {
  BIN_ADD,  // +
  BIN_SUB,  // -
  BIN_MUL,  // *
  BIN_DIV   // /
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
    struct { BinOp op; Node *left; Node *right; } as_binary;
  } u;
};

/* Construtores */
Node *ast_int(long value);
Node *ast_float(double value);
Node *ast_bool(bool value);
Node *ast_binary(BinOp op, Node *left, Node *right);

/* Printer */
void ast_print(const Node *node);
void ast_print_pretty(const Node *node);

/* Liberador de memória */
void ast_free(Node *node);

#endif /* AST_H */
