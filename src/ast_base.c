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
