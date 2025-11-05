/**
 * @file main.c
 * @brief Ponto de entrada do compilador — integra Flex, Bison e AST.
 *
 * Este módulo inicializa o analisador sintático (parser) gerado pelo Bison,
 * conecta o analisador léxico (Flex), executa a análise do código-fonte e,
 * caso bem-sucedida, imprime a Árvore Sintática Abstrata (AST) resultante.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
// #include "semantic_analyzer.h"   // (Passo 2) por enquanto, não precisamos

/* -------------------------------------------------------------------------- */
/* Símbolos exportados pelo parser e scanner (gerados pelo Bison/Flex)        */
/* -------------------------------------------------------------------------- */
extern int yyparse(void);
extern FILE *yyin;
extern int yylex_destroy(void);

/* Se estes não estiverem em ast.h, deixe os extern abaixo. */
extern Node *g_program_ast;
extern int g_parse_errors;

/* -------------------------------------------------------------------------- */
/* Função utilitária: exibe uso do programa                                   */
/* -------------------------------------------------------------------------- */
static void usage(const char *prog) {
    fprintf(stderr, "Uso: %s [arquivo | --]\n", prog);
    fprintf(stderr, "   Se não for informado um arquivo, lê da entrada padrão (stdin).\n");
}

/* -------------------------------------------------------------------------- */
/* Função principal do compilador                                             */
/* -------------------------------------------------------------------------- */
int main(int argc, char **argv) {
    const char *input = NULL;

    /* Etapa 1: argumentos */
    if (argc > 1 && strcmp(argv[1], "-h") == 0) {
        usage(argv[0]);
        return 2;
    }

    if (argc > 1 && strcmp(argv[1], "--") != 0) {
        input = argv[1];
        yyin = fopen(input, "r");
        if (!yyin) {
            perror("Erro ao abrir arquivo");
            return 2;
        }
    } else {
        yyin = stdin; /* Se não houver argumento, lê da entrada padrão */
    }

    /* Etapa 2: inicialização de globais */
    g_program_ast = NULL;
    g_parse_errors = 0;
    /* REMOVIDO: push_scope(); */

    /* Etapa 3: parsing */
    int rc = yyparse();

    if (yyin && yyin != stdin)
        fclose(yyin);

    /* REMOVIDO: while (scope_stack) { pop_scope(); } */

    /* Etapa 4: erros sintáticos */
    if (rc != 0 || g_parse_errors > 0) {
        if (g_program_ast) { ast_free(g_program_ast); g_program_ast = NULL; }
        yylex_destroy();
        return 1;
    }

    /* Etapa 5: saída/limpeza */
    if (g_program_ast) {
        /* Para testes sintáticos, normalmente não imprimimos AST.
           Deixe comentado por enquanto. */
        // printf("=== AST (Compacta) ===\n");
        // ast_print(g_program_ast);
        printf("=== AST (Formatada) ===\n");
        ast_print_pretty(g_program_ast);

        ast_free(g_program_ast);
        g_program_ast = NULL;
    }

    yylex_destroy();
    return 0;
}
