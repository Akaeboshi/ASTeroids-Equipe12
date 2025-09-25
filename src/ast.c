#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (!p) { fprintf(stderr, "error: malloc failed\n"); exit(1); }
  return p;
}

static char *xstrdup(const char *string) {
  size_t n = strlen(string);
  char *p = (char *)xmalloc(n + 1);
  if (!p) { fprintf(stderr, "error: malloc failed\n"); exit(1); }
  memcpy(p, string, n + 1);
  return p;
}

static Node *new_node(NodeKind kind) {
  Node *node = (Node *)xmalloc(sizeof(Node));
  node->kind = kind;
  return node;
}

Node *ast_int(long value) {
  Node *node = new_node(ND_INT);
  node->u.as_int.value = value;
  return node;
}

Node *ast_float(double value) {
  Node *node = new_node(ND_FLOAT);
  node->u.as_float.value = value;
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

Node *ast_binary(BinOp op, Node *left, Node *right) {
  Node *node = new_node(ND_BINARY);
  node->u.as_binary.op = op;
  node->u.as_binary.left = left;
  node->u.as_binary.right = right;
  return node;
}

static const char *binop_to_str(BinOp op) {
  switch (op) {
    case BIN_ADD: return "+";
    case BIN_SUB: return "-";
    case BIN_MUL: return "*";
    case BIN_DIV: return "/";
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
  case ND_BINARY:
    printf("(");
    print(node -> u.as_binary.left);
    printf(" %s ", binop_to_str(node -> u.as_binary.op));
    print(node -> u.as_binary.right);
    printf(")");
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
    case ND_BINARY:
      printf("Binary(%s)\n", binop_to_str(node -> u.as_binary.op));
      print_pretty(node -> u.as_binary.left, depth + 2);
      print_pretty(node -> u.as_binary.right, depth + 2);
      break;
  }
}

void ast_print_pretty(const Node *node) {
  print_pretty(node, 0);
}

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
    case ND_BINARY:
      ast_free(node -> u.as_binary.left);
      ast_free(node -> u.as_binary.right);
      break;
  }

  free(node);
}


