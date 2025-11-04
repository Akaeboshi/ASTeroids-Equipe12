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
#include "semantic_analyzer.h"

/* -------------------------------------------------------------------------- */
/* Símbolos exportados pelo parser e scanner (gerados pelo Bison/Flex)        */
/* -------------------------------------------------------------------------- */
int yyparse(void);
extern FILE *yyin;

/* -------------------------------------------------------------------------- */
/* Variáveis globais definidas em parser.y                                    */
/* -------------------------------------------------------------------------- */
extern Node *g_program_ast;
extern int   g_parse_errors;

/* -------------------------------------------------------------------------- */
/* Escopo global (definido em parser.y)                                       */
/* -------------------------------------------------------------------------- */
extern void push_scope(void);
extern void pop_scope(void);
extern void *scope_stack;

/* -------------------------------------------------------------------------- */
/* Função utilitária: exibe uso do programa                                   */
/* -------------------------------------------------------------------------- */
/**
 * @brief
 *
 * @param prog
 */
static void usage(const char *prog) {
    fprintf(stderr, "Uso: %s [arquivo | --]\n", prog);
    fprintf(stderr, "   Se não for informado um arquivo, lê da entrada padrão (stdin).\n");
}

/* -------------------------------------------------------------------------- */
/* Função principal do compilador                                             */
/* -------------------------------------------------------------------------- */
/**
 * @brief Função principal que executa o pipeline de análise sintática.
 *
 * Fluxo de execução:
 * 1. Determina a fonte de entrada (arquivo ou stdin).
 * 2. Chama o parser (yyparse).
 * 3. Caso não haja erros, imprime a AST gerada.
 * 4. Libera memória antes de encerrar.
 *
 * @param argc Contador de argumentos de linha de comando.
 * @param argv Vetor de argumentos de linha de comando.
 * @return int Código de saída (0 = sucesso, 1 = erro de sintaxe, 2 = erro de I/O).
 */
int main(int argc, char **argv) {
    const char *input = NULL;

    /* ---------------------------------------------------------------------- */
    /* Etapa 1: Processamento de argumentos                                   */
    /* ---------------------------------------------------------------------- */
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
        yyin = stdin; /* Se não houver argumento, lê código da entrada padrão */
    }

    /* ---------------------------------------------------------------------- */
    /* Etapa 2: Inicialização de variáveis globais                            */
    /* ---------------------------------------------------------------------- */
    g_program_ast = NULL;
    g_parse_errors = 0;
    push_scope();

    /* ---------------------------------------------------------------------- */
    /* Etapa 3: Execução da análise sintática                                 */
    /* ---------------------------------------------------------------------- */
    int rc = yyparse();

    if (yyin && yyin != stdin)
        fclose(yyin);

    while (scope_stack) {
        pop_scope();
    }

    /* ---------------------------------------------------------------------- */
    /* Etapa 4: Tratamento de erros sintáticos                                */
    /* ---------------------------------------------------------------------- */
    if (rc != 0 || g_parse_errors > 0) {
        /* A função yyerror() já exibe mensagens detalhadas. */
        return 1;
    }

    /* ---------------------------------------------------------------------- */
    /* Etapa 5: Impressão e liberação da AST                                  */
    /* ---------------------------------------------------------------------- */
    if (g_program_ast) {
        printf("=== AST (Compacta) ===\n");
        ast_print(g_program_ast,0);

        printf("=== AST (Formatada) ===\n");
        ast_print_pretty(g_program_ast);

        ast_free(g_program_ast);
        g_program_ast = NULL;
    }

    return 0;
}
