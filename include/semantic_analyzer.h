#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast_base.h"
#include "symbol_table.h"

// Enum para seus tipos básicos
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_ERROR // Um tipo especial para propagar erros
} Type;

/* Executa a análise semântica sobre a AST.
 * Retorna: número de erros encontrados (0 = sucesso).
 */
int check_semantics(Node *root, SymbolTable *table);

/* Retorna 1 se não há erros, 0 se há erros. */
int semantics_ok(Node *root, SymbolTable *table);


#endif
