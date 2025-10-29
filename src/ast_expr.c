#include "ast_base.h"
#include "ast_expr.h"

#include <stdio.h>
#include <stdlib.h>

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

Node *ast_string(const char *value) {
  Node *node = new_node(ND_STRING);
  node -> u.as_string.value = xstrdup(value);
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

Node *ast_block(void) {
  Node *node = new_node(ND_BLOCK);
  node -> u.as_block.stmts = NULL;
  node -> u.as_block.count = 0;
  node -> u.as_block.capacity = 0;
  return node;
}

void ast_block_add_stmt(Node *block, Node *stmt) {
  if (block -> kind != ND_BLOCK) return;

  size_t count = block -> u.as_block.count;
  size_t capacity = block -> u.as_block.capacity;

  if (count == capacity) {
    size_t new_capacity = capacity ? capacity * 2 : 4;

    Node **new_stmts = (Node **)realloc(block -> u.as_block.stmts, new_capacity * sizeof(Node *));

    if (!new_stmts) { fprintf(stderr, "Erro de alocação de memória\n"); exit(1); }

    block -> u.as_block.stmts = new_stmts;
    block -> u.as_block.capacity = new_capacity;
  }

  block -> u.as_block.stmts[block -> u.as_block.count++] = stmt;
}

Node *ast_assign(const char *name, Node *value) {
  Node *node = new_node(ND_ASSIGN);
  node -> u.as_assign.name = xstrdup(name);
  node -> u.as_assign.value = value;
  return node;
}

Node *ast_expr(Node *expr) {
  Node *node = new_node(ND_EXPR);
  node -> u.as_expr.expr = expr;
  return node;
}

Node *ast_if(Node *cond, Node *then_branch, Node *else_branch) {
  Node *node = new_node(ND_IF);
  node -> u.as_if.cond = cond;
  node -> u.as_if.then_branch = then_branch;
  node -> u.as_if.else_branch = else_branch;
  return node;
}

Node *ast_decl(TypeTag type, const char *name, Node *init) {
  Node *n = new_node(ND_DECL);
  n->u.as_decl.type = type;
  n->u.as_decl.name = xstrdup(name);
  n->u.as_decl.init = init;
  return n;
}

Node *ast_while(Node *cond, Node *body) {
    Node *n = xmalloc(sizeof(Node));
    n->kind = ND_WHILE;
    n->u.as_while.cond = cond;
    n->u.as_while.body = body;
    return n;
}

Node *ast_for(Node *init, Node *cond, Node *step, Node *body) {
    Node *n = xmalloc(sizeof(Node));
    n->kind = ND_FOR;
    n->u.as_for.init = init;
    n->u.as_for.cond = cond;
    n->u.as_for.step = step;
    n->u.as_for.body = body;
    return n;
}
