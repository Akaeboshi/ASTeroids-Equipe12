#include "ast_printer.h"

#include <stdio.h>

static const char *binop_to_str(BinOp op) {
  switch (op) {
    case BIN_ADD: return "+";
    case BIN_SUB: return "-";
    case BIN_MUL: return "*";
    case BIN_DIV: return "/";
    case BIN_EQ:  return "==";
    case BIN_NEQ: return "!=";
    case BIN_LT:  return "<";
    case BIN_LE:  return "<=";
    case BIN_GT:  return ">";
    case BIN_GE:  return ">=";
    case BIN_AND: return "&&";
    case BIN_OR:  return "||";
    default: return "?";
  }
}

static const char *unop_to_str(UnOp op) {
  switch (op) {
    case UN_NEG: return "-";
    case UN_NOT: return "!";
    default: return "?";
  }
}

static const char *type_to_string(TypeTag type) {
    switch (type) {
        case TY_INT:    return "int";
        case TY_FLOAT:  return "float";
        case TY_BOOL:   return "bool";
        case TY_STRING: return "string";
        case TY_VOID:   return "void";
        default:        return "<invalid>";
    }
}

static void print (const Node *node) {
if(!node) { printf("NULL"); return; }
  switch (node -> kind) {
    case ND_INT:
      printf("%ld", node -> u.as_int.value);
      break;

    case ND_FLOAT:
      printf("%lf", node -> u.as_float.value);
      break;

    case ND_BOOL:
      printf("%s", node -> u.as_bool.value ? "true" : "false");
      break;

    case ND_IDENT:
      printf("%s", node -> u.as_ident.name);
      break;

    case ND_STRING:
      printf("%s", node -> u.as_string.value);
      break;

    case ND_UNARY:
      printf("(");
      printf("%s", unop_to_str(node -> u.as_unary.op));
      print(node -> u.as_unary.expr);
      printf(")");
      break;

    case ND_BINARY:
      printf("(");
      print(node -> u.as_binary.left);
      printf(" %s ", binop_to_str(node -> u.as_binary.op));
      print(node -> u.as_binary.right);
      printf(")");
      break;

    case ND_BLOCK:
    printf("{ ");

    for (size_t i = 0; i < node -> u.as_block.count; i++){
      print(node -> u.as_block.stmts[i]);
      if (i + 1 < node -> u.as_block.count) printf(" ");
    }

    printf(" }");
    break;

    case ND_ASSIGN:
      printf("(%s = ", node -> u.as_assign.name);
      print(node -> u.as_assign.value);
      printf(")");
      break;

    case ND_EXPR:
      print(node -> u.as_expr.expr);
      printf(";");
      break;

    case ND_IF:
      printf("if (");
      print(node -> u.as_if.cond);
      printf(") ");
      print(node -> u.as_if.then_branch);
      if (node -> u.as_if.else_branch) {
        printf(" else ");
        print(node -> u.as_if.else_branch);
      }
      break;

    case ND_DECL: {
      if (node->u.as_decl.init) {
        printf("%s %s = ", type_to_string(node->u.as_decl.type), node->u.as_decl.name);
        print(node->u.as_decl.init);
      } else {
        printf("%s %s", type_to_string(node->u.as_decl.type), node->u.as_decl.name);
      }
      break;
    }

    case ND_WHILE:
      printf("while (");
      print(node->u.as_while.cond);
      printf(") ");
      print(node->u.as_while.body);
      break;

    case ND_FOR:
      printf("for (");
      print(node->u.as_for.init);
      printf("; ");
      print(node->u.as_for.cond);
      printf("; ");
      print(node->u.as_for.step);
      printf(") ");
      print(node->u.as_for.body);
      break;

    case ND_FUNCTION: {
      printf("%s %s(", type_to_string(node->u.as_function.ret_type),
                      node->u.as_function.name);
      for (size_t i = 0; i < node->u.as_function.param_count; i++) {
        Node *p = node->u.as_function.params[i]; /* ND_DECL */
        printf("%s %s", type_to_string(p->u.as_decl.type), p->u.as_decl.name);
        if (i + 1 < node->u.as_function.param_count) printf(", ");
      }
      printf(") ");
      print(node->u.as_function.body);
      break;
    }

    case ND_RETURN:
      printf("return");
      if (node->u.as_return.expr) {
        printf(" ");
        print(node->u.as_return.expr);
      }
      printf(";");
      break;

    case ND_CALL: {
      printf("%s(", node->u.as_call.name);
      for (size_t i = 0; i < node->u.as_call.arg_count; i++) {
        print(node->u.as_call.args[i]);
        if (i + 1 < node->u.as_call.arg_count) printf(", ");
      }
      printf(")");
      break;
    }

  }
}

void ast_print(const Node *node) {
  print(node);
  printf("\n");
}

static void indent(int depth) {
  for (int i = 0; i < depth; i++) putchar(' ');
}

static void print_pretty(const Node *node, int depth) {
  if (!node) {
    indent(depth);
    printf("<null>\n");
    return;
  }

  indent(depth);
  switch (node->kind) {
    case ND_INT:
      printf("Int(%ld)\n", node->u.as_int.value);
      break;

    case ND_FLOAT:
      printf("Float(%g)\n", node->u.as_float.value);
      break;

    case ND_BOOL:
      printf("Bool(%s)\n", node->u.as_bool.value ? "true" : "false");
      break;

    case ND_IDENT:
      printf("Ident(%s)\n", node->u.as_ident.name);
      break;

    case ND_STRING:
      printf("String(%s)\n", node->u.as_string.value);
      break;

    case ND_UNARY:
      printf("Unary(%s)\n", unop_to_str(node->u.as_unary.op));
      print_pretty(node->u.as_unary.expr, depth + 2);
      break;

    case ND_BINARY:
      printf("Binary(%s)\n", binop_to_str(node->u.as_binary.op));
      print_pretty(node->u.as_binary.left,  depth + 2);
      print_pretty(node->u.as_binary.right, depth + 2);
      break;

    case ND_BLOCK:
      printf("Block\n");
      for (size_t i = 0; i < node->u.as_block.count; i++) {
        print_pretty(node->u.as_block.stmts[i], depth + 2);
      }
      break;

    case ND_ASSIGN:
      printf("Assign(%s)\n", node->u.as_assign.name);
      print_pretty(node->u.as_assign.value, depth + 2);
      break;

    case ND_EXPR:
      printf("Expression\n");
      print_pretty(node->u.as_expr.expr, depth + 2);
      break;

    case ND_IF:
      printf("If\n");
      indent(depth + 2); printf("Cond:\n");
      print_pretty(node->u.as_if.cond, depth + 4);
      indent(depth + 2); printf("Then:\n");
      print_pretty(node->u.as_if.then_branch, depth + 4);
      if (node->u.as_if.else_branch) {
        indent(depth + 2); printf("Else:\n");
        print_pretty(node->u.as_if.else_branch, depth + 4);
      }
      break;

    case ND_DECL:
      printf("Decl(%s %s)\n", type_to_string(node->u.as_decl.type), node->u.as_decl.name);
      if (node->u.as_decl.init) {
        indent(depth + 2); printf("Init:\n");
        print_pretty(node->u.as_decl.init, depth + 4);
      }
      break;

    case ND_WHILE:
      printf("While\n");
      indent(depth + 2); printf("Cond:\n");
      print_pretty(node->u.as_while.cond, depth + 4);
      indent(depth + 2); printf("Body:\n");
      print_pretty(node->u.as_while.body, depth + 4);
      break;

    case ND_FOR:
      printf("For\n");
      indent(depth + 2); printf("Init:\n");
      print_pretty(node->u.as_for.init, depth + 4);
      indent(depth + 2); printf("Cond:\n");
      print_pretty(node->u.as_for.cond, depth + 4);
      indent(depth + 2); printf("Step:\n");
      print_pretty(node->u.as_for.step, depth + 4);
      indent(depth + 2); printf("Body:\n");
      print_pretty(node->u.as_for.body, depth + 4);
      break;

    case ND_FUNCTION:
      printf("Function(%s %s)\n", type_to_string(node->u.as_function.ret_type), node->u.as_function.name);
      for (int i = 0; i < depth + 2; i++) putchar(' ');
      printf("Params:\n");
      for (size_t i = 0; i < node->u.as_function.param_count; i++) {
        print_pretty(node->u.as_function.params[i], depth + 4);
      }
      for (int i = 0; i < depth + 2; i++) putchar(' ');
      printf("Body:\n");
      print_pretty(node->u.as_function.body, depth + 4);
      break;

    case ND_RETURN:
      printf("Return\n");
      if (node->u.as_return.expr) {
        for (int i = 0; i < depth + 2; i++) putchar(' ');
        printf("Expr:\n");
        print_pretty(node->u.as_return.expr, depth + 4);
      }
      break;

    case ND_CALL:
      printf("Call(%s)\n", node->u.as_call.name);
      if (node->u.as_call.arg_count > 0) {
        indent(depth + 2); printf("Args:\n");
        for (size_t i = 0; i < node->u.as_call.arg_count; i++) {
          print_pretty(node->u.as_call.args[i], depth + 4);
        }
      }
      break;
  }
}

void ast_print_pretty(const Node *node) {
  print_pretty(node, 0);
}
