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

/* Versão Compacta */
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
  }
}

void ast_print(const Node *node) {
  print(node);
  printf("\n");
}

/* Versão Indentada */
static void print_pretty(const Node *node, int depth) {
  if (!node) {
    for (int i = 0; i < depth; i++) putchar(' ');
    printf("<null>\n");
    return;
  }

  for (int i = 0; i < depth; i++) putchar(' ');

  switch (node -> kind) {
    case ND_INT:
      printf("Int(%ld)\n", node -> u.as_int.value);
      break;
    case ND_FLOAT:
      printf("Float(%g)\n", node -> u.as_float.value);
      break;
    case ND_BOOL:
      printf("Bool(%s)\n", node -> u.as_bool.value ? "true" : "false");
      break;
    case ND_IDENT:
      printf("Ident(%s)\n", node -> u.as_ident.name);
      break;
    case ND_STRING:
      printf("String(%s)\n", node -> u.as_string.value);
      break;
    case ND_UNARY:
      printf("Unary(%s)\n", unop_to_str(node -> u.as_unary.op));
      print_pretty(node -> u.as_unary.expr, depth + 2);
      break;
    case ND_BINARY:
      printf("Binary(%s)\n", binop_to_str(node -> u.as_binary.op));
      print_pretty(node -> u.as_binary.left, depth + 2);
      print_pretty(node -> u.as_binary.right, depth + 2);
      break;
      case ND_BLOCK:
      printf("Block\n");
      for (size_t i = 0; i < node -> u.as_block.count; i++) {
        print_pretty(node -> u.as_block.stmts[i], depth + 2);
      }
      break;
      case ND_ASSIGN:
        printf("Assign(%s)\n", node -> u.as_assign.name);
        print_pretty(node -> u.as_assign.value, depth + 2);
        break;
      case ND_EXPR:
        printf("Expression\n");
        print_pretty(node -> u.as_expr.expr, depth + 2);
        break;
  }
}

void ast_print_pretty(const Node *node) {
  print_pretty(node, 0);
}
