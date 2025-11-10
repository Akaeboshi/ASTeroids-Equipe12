// symbol_table.h
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast_base.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct Symbol {
    char *name;
    TypeTag type;
    Node *value;
    struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
    Symbol **buckets;
    size_t size;
    size_t capacity;
    struct SymbolTable *parent; // para escopos
} SymbolTable;

// Cria uma nova tabela de símbolos
SymbolTable* st_create(void);

// Destroi a tabela e todos os símbolos
void st_destroy(SymbolTable *table);

// Insere um símbolo na tabela (sobrescreve se existir)
bool st_insert(SymbolTable *table, const char *name, TypeTag type, Node *value);

// Busca um símbolo na tabela (apenas no escopo atual)
Node* st_lookup(SymbolTable *table, const char *name);

// Busca recursiva em todos os escopos (para escopos aninhados)
Node* st_lookup_recursive(SymbolTable *table, const char *name);

// Remove um símbolo da tabela
bool st_remove(SymbolTable *table, const char *name);

// Debug: imprime toda a tabela
void st_print(SymbolTable *table);

// Atualiza o valor de um símbolo existente no escopo atual
bool st_update(SymbolTable *table, const char *name, Node *new_value);

// Atualiza o valor de um símbolo existente recursivamente
bool st_update_recursive(SymbolTable *table, const char *name, Node *new_value);

// Busca o tipo de um símbolo no escopo atual
TypeTag st_lookup_type(SymbolTable *table, const char *name, bool *found);

// Busca recursiva o tipo de um símbolo em todos os escopos
TypeTag st_lookup_type_recursive(SymbolTable *table, const char *name, bool *found);

/* Helper Functions */
void *xmalloc(size_t size);
char *xstrdup(const char *s);

#endif
