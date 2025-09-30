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
  }

  free(node);
}
