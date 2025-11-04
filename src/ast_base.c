#include "ast_base.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (!p) { fprintf(stderr, "error: malloc failed\n"); exit(1); }
  return p;
}

char *xstrdup(const char *string) {
  size_t n = strlen(string);
  char *p = (char *)xmalloc(n + 1);
  memcpy(p, string, n + 1);
  return p;
}

Node *new_node(NodeKind kind) {
  struct Node *node = (struct Node *)xmalloc(sizeof(struct Node));
  node->kind = kind;
  return node;
}

Node *ast_copy(Node *node) {
    if (!node) return NULL;

    Node *copy = new_node(node->kind);

    switch (node->kind) {
      case ND_INT:
        copy->u.as_int.value = node->u.as_int.value;
        break;

      case ND_FLOAT:
        copy->u.as_float.value = node->u.as_float.value;
        break;

      case ND_BOOL:
        copy->u.as_bool.value = node->u.as_bool.value;
        break;

      case ND_IDENT:
        copy->u.as_ident.name = xstrdup(node->u.as_ident.name);
        break;

      case ND_STRING:
        copy->u.as_string.value = xstrdup(node->u.as_string.value);
        break;

      case ND_UNARY:
        copy->u.as_unary.op   = node->u.as_unary.op;
        copy->u.as_unary.expr = ast_copy(node->u.as_unary.expr);
        break;

      case ND_BINARY:
        copy->u.as_binary.op    = node->u.as_binary.op;
        copy->u.as_binary.left  = ast_copy(node->u.as_binary.left);
        copy->u.as_binary.right = ast_copy(node->u.as_binary.right);
        break;

      case ND_BLOCK:
        copy->u.as_block.count = node->u.as_block.count;
        copy->u.as_block.stmts = xmalloc(sizeof(Node*) * node->u.as_block.count);
        for (size_t i = 0; i < node->u.as_block.count; i++)
            copy->u.as_block.stmts[i] = ast_copy(node->u.as_block.stmts[i]);
        break;

      case ND_ASSIGN:
        copy->u.as_assign.name  = xstrdup(node->u.as_assign.name);
        copy->u.as_assign.value = ast_copy(node->u.as_assign.value);
        break;

      case ND_EXPR:
        copy->u.as_expr.expr = ast_copy(node->u.as_expr.expr);
        break;

      case ND_IF:
        copy->u.as_if.cond        = ast_copy(node->u.as_if.cond);
        copy->u.as_if.then_branch = ast_copy(node->u.as_if.then_branch);
        copy->u.as_if.else_branch = ast_copy(node->u.as_if.else_branch);
        break;

      case ND_DECL:
        copy->u.as_decl.type = node->u.as_decl.type;
        copy->u.as_decl.name = xstrdup(node->u.as_decl.name);
        copy->u.as_decl.init = node->u.as_decl.init ? ast_copy(node->u.as_decl.init) : NULL;
        break;

      case ND_WHILE:
        copy->u.as_while.cond = ast_copy(node->u.as_while.cond);
        copy->u.as_while.body = ast_copy(node->u.as_while.body);
        break;

      case ND_FOR:
        copy->u.as_for.init = ast_copy(node->u.as_for.init);
        copy->u.as_for.cond = ast_copy(node->u.as_for.cond);
        copy->u.as_for.step = ast_copy(node->u.as_for.step);
        copy->u.as_for.body = ast_copy(node->u.as_for.body);
        break;

      case ND_FUNCTION:
        copy->u.as_function.ret_type    = node->u.as_function.ret_type;
        copy->u.as_function.name        = xstrdup(node->u.as_function.name);
        copy->u.as_function.param_count = node->u.as_function.param_count;

        if (node->u.as_function.param_count > 0) {
            copy->u.as_function.params = xmalloc(sizeof(Node*) * node->u.as_function.param_count);
            for (size_t i = 0; i < node->u.as_function.param_count; i++)
                copy->u.as_function.params[i] = ast_copy(node->u.as_function.params[i]);
        } else {
            copy->u.as_function.params = NULL;
        }

        copy->u.as_function.body = ast_copy(node->u.as_function.body);
        break;

      case ND_RETURN:
        copy->u.as_return.expr = ast_copy(node->u.as_return.expr);
        break;

      case ND_CALL:
        copy->u.as_call.name = xstrdup(node->u.as_call.name);
        copy->u.as_call.args = ast_copy(node->u.as_call.args);
        break;
    }

    return copy;
}
