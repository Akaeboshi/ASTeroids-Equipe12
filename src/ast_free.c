#include "ast_free.h"
#include <stdlib.h>

void ast_free(Node *node) {
  if (!node) return;

  switch (node -> kind) {
    case ND_INT:
      break;

    case ND_FLOAT:
      break;

    case ND_BOOL:
      break;

    case ND_IDENT:
      free(node -> u.as_ident.name);
      break;

    case ND_STRING:
      free(node -> u.as_string.value);
      break;

    case ND_UNARY:
      ast_free(node -> u.as_unary.expr);
      break;

    case ND_BINARY:
      ast_free(node -> u.as_binary.left);
      ast_free(node -> u.as_binary.right);
      break;

    case ND_BLOCK:
      for (size_t i = 0; i < node -> u.as_block.count; i++) {
        ast_free(node -> u.as_block.stmts[i]);
      }
      free(node -> u.as_block.stmts);
      break;

    case ND_ASSIGN:
      free(node -> u.as_assign.name);
      ast_free(node -> u.as_assign.value);
      break;

    case ND_EXPR:
      ast_free(node -> u.as_expr.expr);
      break;

    case ND_IF:
      ast_free(node -> u.as_if.cond);
      ast_free(node -> u.as_if.then_branch);
      if (node->u.as_if.else_branch) ast_free(node->u.as_if.else_branch);
      break;

    case ND_DECL:
      free(node->u.as_decl.name);
      if (node->u.as_decl.init) ast_free(node->u.as_decl.init);
      break;

    case ND_WHILE:
      if (node->u.as_while.cond) ast_free(node->u.as_while.cond);
      if (node->u.as_while.body) ast_free(node->u.as_while.body);
      break;

    case ND_FOR:
      if (node->u.as_for.init) ast_free(node->u.as_for.init);
      if (node->u.as_for.cond) ast_free(node->u.as_for.cond);
      if (node->u.as_for.step) ast_free(node->u.as_for.step);
      if (node->u.as_for.body) ast_free(node->u.as_for.body);
      break;

    case ND_FUNCTION: {
      free(node->u.as_function.name);
      for (size_t i = 0; i < node->u.as_function.param_count; i++) {
          ast_free(node->u.as_function.params[i]);
      }
      free(node->u.as_function.params);
      ast_free(node->u.as_function.body);
      break;
    }

    case ND_RETURN:
      if (node->u.as_return.expr) ast_free(node->u.as_return.expr);
      break;

    case ND_CALL:
      if(node->u.as_call.name)  free(node->u.as_call.name);
      if (node->u.as_call.args) ast_free(node->u.as_call.args);
      break;
  }

  free(node);
}
