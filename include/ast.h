#ifndef AST_H
#define AST_H

#include "ast_base.h"
#include "ast_expr.h"
#include "ast_printer.h"
#include "ast_free.h"

#endif /* AST_H */

enum NodeType {
    NODE_INT,
    NODE_FLOAT,
    NODE_BOOL,
    NODE_IDENT,
    NODE_STRING,   /* novo */
    NODE_BINARY,
    NODE_UNARY
};

struct Node {
    enum NodeType type;
    union {
        int intValue;
        double floatValue;
        int boolValue;
        char *ident;
        char *stringValue; /* novo */
    };
    struct Node *left, *right;
};

/* novas funções */
struct Node* ast_string(const char *value);

