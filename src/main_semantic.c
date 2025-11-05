#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "semantic_analyzer.h"
#include "symbol_table.h"

extern int yyparse(void);
extern FILE *yyin;
extern int yylex_destroy(void);
extern Node *g_program_ast;
extern int g_parse_errors;

static void usage(const char *prog) {
    fprintf(stderr, "Uso: %s [arquivo | --]\n", prog);
}

int main(int argc, char **argv) {
    const char *path = NULL;
    if (argc > 1 && strcmp(argv[1], "-h") == 0) { usage(argv[0]); return 2; }
    if (argc > 1 && strcmp(argv[1], "--") != 0) {
        path = argv[1];
        yyin = fopen(path, "r");
        if (!yyin) { perror("Erro ao abrir arquivo"); return 2; }
    } else {
        yyin = stdin;
    }

    g_program_ast = NULL;
    g_parse_errors = 0;

    int rc = yyparse();
    if (yyin && yyin != stdin) fclose(yyin);

    if (rc != 0 || g_parse_errors > 0) {
        if (g_program_ast) { ast_free(g_program_ast); g_program_ast = NULL; }
        yylex_destroy();
        return 1; /* erro de sintaxe */
    }

    SymbolTable *global = st_create();
    int sem_errors = check_semantics(g_program_ast, global);
    st_destroy(global);

    if (g_program_ast) { ast_free(g_program_ast); g_program_ast = NULL; }
    yylex_destroy();

    return (sem_errors == 0) ? 0 : 1;
}
