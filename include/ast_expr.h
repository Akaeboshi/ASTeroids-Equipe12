#ifndef AST_EXPR_H
#define AST_EXPR_H

#include "ast_base.h"

Node *ast_int(long value);
Node *ast_float(double value);
Node *ast_bool(bool value);
Node *ast_ident(const char *name);
Node *ast_binary(BinOp op, Node *left, Node *right);

#endif /* AST_EXPR_H */
