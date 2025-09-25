#include "ast_base.h"
#include "ast_expr.h"

Node *ast_int(long value) {
  Node *node = new_node(ND_INT);
  node -> u.as_int.value = value;
  return node;
}

Node *ast_float(double value) {
  Node *node = new_node(ND_FLOAT);
  node -> u.as_float.value = value;
  return node;
}

Node *ast_bool(bool value) {
  Node *node = new_node(ND_BOOL);
  node -> u.as_bool.value = value;
  return node;
}

Node *ast_ident(const char *name) {
  Node *node = new_node(ND_IDENT);
  node -> u.as_ident.name = xstrdup(name);
  return node;
}

Node *ast_unary(UnOp op, Node *expr) {
  Node *node = new_node(ND_UNARY);
  node -> u.as_unary.op = op;
  node -> u.as_unary.expr = expr;
  return node;
}

Node *ast_binary(BinOp op, Node *left, Node *right) {
  Node *node = new_node(ND_BINARY);
  node -> u.as_binary.op = op;
  node -> u.as_binary.left = left;
  node -> u.as_binary.right = right;
  return node;
}
