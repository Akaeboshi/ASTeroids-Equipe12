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

/* * A função principal.
 * Ela "visita" a AST, preenche a tabela de símbolos e checa os tipos.
 * Retorna 1 (true) se tudo estiver correto, 0 (false) se houver erros.
 */
int check_semantics(Node* ast_root, SymbolTable* table);

#endif
