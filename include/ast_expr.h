#ifndef AST_EXPR_H
#define AST_EXPR_H

#include "ast_base.h"

Node *ast_int(long value);
Node *ast_float(double value);
Node *ast_bool(bool value);
Node *ast_ident(const char *name);
Node *ast_string(const char *value);
Node *ast_unary(UnOp op, Node *expr);
Node *ast_binary(BinOp op, Node *left, Node *right);
Node *ast_block(void);
void ast_block_add_stmt(Node *block, Node *stmt);
Node *ast_assign(const char *name, Node *value);
Node *ast_expr(Node *expr);
Node *ast_if(Node *cond, Node *then_branch, Node *else_branch);

#endif /* AST_EXPR_H */
