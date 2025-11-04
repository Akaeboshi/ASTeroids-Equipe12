#include "semantic_analyzer.h"
#include "ast_expr.h" // Para acessar os nós de expressão
#include <stdio.h>

// Função auxiliar recursiva que faz o trabalho real
// Retorna o 'Type' da expressão/statement
static Type visit_node(ASTNode* node, SymbolTable* table) {
    switch(node->type_tag) {
        
        case NODE_INT_LITERAL: {
            return TYPE_INT; // Inferência: Literais inteiros são do tipo INT
        }
        
        case NODE_BOOL_LITERAL: {
            return TYPE_BOOL; // Inferência: Literais booleanos são BOOL
        }
        
        case NODE_VARIABLE: {
            // Checagem: Uso sem declaração
            Symbol* sym = symbol_table_lookup(table, node->var.name);
            if (sym == NULL) {
                fprintf(stderr, "Erro Semântico: Variável '%s' não declarada.\n", node->var.name);
                return TYPE_ERROR;
            }
            return sym->type; // Retorna o tipo armazenado
        }

        case NODE_ASSIGN: {
            // 1. Visita o lado direito para inferir seu tipo
            Type expr_type = visit_node(node->assign.value, table);
            if (expr_type == TYPE_ERROR) return TYPE_ERROR;

            // 2. Checa/Inferência do lado esquerdo
            Symbol* sym = symbol_table_lookup(table, node->assign.target_name);
            if (sym == NULL) {
                // Inferência: Primeira vez que vemos, declaramos
                symbol_table_declare(table, node->assign.target_name, expr_type);
                return expr_type;
            } else {
                // Checagem: Variável já existe. Os tipos batem?
                if (sym->type != expr_type) {
                    fprintf(stderr, "Erro de Tipo: Incompatível. Variável '%s' é %d, mas recebeu %d.\n", 
                            node->assign.target_name, sym->type, expr_type);
                    return TYPE_ERROR;
                }
                return sym->type;
            }
        }
        
        case NODE_IF: {
            Type cond_type = visit_node(node->if_stmt.condition, table);
            if (cond_type != TYPE_BOOL) {
                 fprintf(stderr, "Erro de Tipo: 'if' espera BOOL, mas recebeu %d.\n", cond_type);
                 return TYPE_ERROR;
            }
            visit_node(node->if_stmt.then_block, table);
            if (node->if_stmt.else_block) {
                visit_node(node->if_stmt.else_block, table);
            }
            return TYPE_VOID; // 'if' é um statement, não tem valor
        }

        // ... outros casos para BIN_OP, FUNC_CALL, BLOCK ...
        
        case NODE_BLOCK: {
            symbol_table_enter_scope(table); // IMPORTANTE!
            
            for (int i = 0; i < node->block.statement_count; i++) {
                visit_node(node->block.statements[i], table);
            }

            symbol_table_exit_scope(table); // IMPORTANTE!
            return TYPE_VOID;
        }

        default:
            // Implementar outros nós
            return TYPE_VOID;
    }
}

// Esta é a função pública definida no .h
int check_semantics(ASTNode* ast_root, SymbolTable* table) {
    if (visit_node(ast_root, table) == TYPE_ERROR) {
        return 0; // Falhou
    }
    return 1; // Sucesso
}