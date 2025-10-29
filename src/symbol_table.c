// symbol_table.c
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16

// Função hash simples para strings
static size_t hash(const char *str, size_t capacity) {
    size_t hash_value = 5381;
    int c;
    while ((c = *str++)) {
        hash_value = ((hash_value << 5) + hash_value) + c; // hash * 33 + c
    }
    return hash_value % capacity;
}

SymbolTable* st_create(void) {
    SymbolTable *table = (SymbolTable*)xmalloc(sizeof(SymbolTable));
    table->capacity = INITIAL_CAPACITY;
    table->size = 0;
    table->parent = NULL;
    table->buckets = (Symbol**)xmalloc(table->capacity * sizeof(Symbol*));

    for (size_t i = 0; i < table->capacity; i++) {
        table->buckets[i] = NULL;
    }
    return table;
}

void st_destroy(SymbolTable *table) {
    if (!table) return;

    for (size_t i = 0; i < table->capacity; i++) {
        Symbol *current = table->buckets[i];
        while (current) {
            Symbol *next = current->next;
            free(current->name);
            ast_free(current->value);
            free(current);
            current = next;
        }
    }
    free(table->buckets);
    free(table);
}

bool st_insert(SymbolTable *table, const char *name, TypeTag type, Node *value) {
    if (!table || !name) return false;

    size_t index = hash(name, table->capacity);
    Symbol *current = table->buckets[index];

    // Verifica se já existe
    while (current) {
        if (strcmp(current->name, name) == 0) {
            // Atualiza tipo e valor existentes
            current->type = type;
            ast_free(current->value);
            current->value = value;
            return true;
        }
        current = current->next;
    }

    // Cria novo símbolo
    Symbol *new_symbol = (Symbol*)xmalloc(sizeof(Symbol));
    new_symbol->name = xstrdup(name);
    new_symbol->type = type;
    new_symbol->value = value;
    new_symbol->next = table->buckets[index];
    table->buckets[index] = new_symbol;
    table->size++;

    return true;
}

Node* st_lookup(SymbolTable *table, const char *name) {
    if (!table || !name) return NULL;

    size_t index = hash(name, table->capacity);
    Symbol *current = table->buckets[index];

    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

Node* st_lookup_recursive(SymbolTable *table, const char *name) {
    SymbolTable *current = table;
    while (current) {
        Node *result = st_lookup(current, name);
        if (result) return result;
        current = current->parent;
    }
    return NULL;
}

bool st_remove(SymbolTable *table, const char *name) {
    if (!table || !name) return false;

    size_t index = hash(name, table->capacity);
    Symbol *current = table->buckets[index];
    Symbol *prev = NULL;

    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                table->buckets[index] = current->next;
            }
            free(current->name);
            ast_free(current->value);
            free(current);
            table->size--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

void st_print(SymbolTable *table) {
    if (!table) {
        printf("Tabela de símbolos: NULL\n");
        return;
    }

    printf("Tabela de símbolos (%zu símbolos):\n", table->size);
    for (size_t i = 0; i < table->capacity; i++) {
        Symbol *current = table->buckets[i];
        if (current) {
            printf("  [%zu]: ", i);
            while (current) {
                printf("%s -> ", current->name);
                current = current->next;
            }
            printf("NULL\n");
        }
    }
}

bool st_update(SymbolTable *table, const char *name, Node *new_value) {
    if (!table || !name) return false;
    size_t index = hash(name, table->capacity);
    for (Symbol *cur = table->buckets[index]; cur; cur = cur->next) {
        if (strcmp(cur->name, name) == 0) {
            ast_free(cur->value);
            cur->value = new_value;
            return true;
        }
    }
    return false;
}

bool st_update_recursive(SymbolTable *table, const char *name, Node *new_value) {
    if (!table || !name) return false;

    size_t index = hash(name, table->capacity);
    Symbol *current = table->buckets[index];

    while (current) {
        if (strcmp(current->name, name) == 0) {
            ast_free(current->value);
            current->value = new_value;
            return true;
        }
        current = current->next;
    }

    if (table->parent) {
        return st_update_recursive(table->parent, name, new_value);
    }

    return false;
}

TypeTag st_lookup_type(SymbolTable *table, const char *name, bool *found) {
    if (!table || !name) {
        if (found) *found = false;
        return TY_INVALID;
    }
    if (found) *found = false;
    size_t index = hash(name, table->capacity);
    for (Symbol *cur = table->buckets[index]; cur; cur = cur->next) {
        if (strcmp(cur->name, name) == 0) {
            if (found) *found = true;
            return cur->type;
        }
    }
    return TY_INVALID;
}

TypeTag st_lookup_type_recursive(SymbolTable *table, const char *name, bool *found) {
    if (found) *found = false;
    for (SymbolTable *t = table; t; t = t->parent) {
        size_t index = hash(name, t->capacity);
        for (Symbol *cur = t->buckets[index]; cur; cur = cur->next) {
            if (strcmp(cur->name, name) == 0) {
                if (found) *found = true;
                return cur->type;
            }
        }
    }
    return TY_INVALID;
}
