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
    case ND_UNARY:
      ast_free(node -> u.as_unary.expr);
      break;
    case ND_BINARY:
      ast_free(node -> u.as_binary.left);
      ast_free(node -> u.as_binary.right);
      break;
  }

  free(node);
}
