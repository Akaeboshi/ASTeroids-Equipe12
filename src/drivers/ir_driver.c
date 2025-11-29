#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ast.h"
#include "ir.h"
#include "ir_builder.h"
#include "ir_printer.h"
#include "syntax_analyzer.h"
#include "semantic_analyzer.h"

extern int g_parse_errors;
extern Node *g_program_ast;

static bool read_from_stdin(const char *arg) {
    if (!arg) return true;
    return (strcmp(arg, "-") == 0 || strcmp(arg, "--") == 0);
}

int main(int argc, char **argv) {
    const char *path = NULL;
    if (argc > 1 && !read_from_stdin(argv[1])) {
        path = argv[1];
    }

    // 1) Sintaxe
    SyntaxResult sr = syntax_parse_path(path);
    if (!sr.parse_ok || sr.ast == NULL || g_parse_errors > 0) {
        fprintf(stderr, "IR: abortado por erro(s) sintáticos.\n");
        return 1;
    }

    // 2) Semântica
    SymbolTable *global = st_create();
    int ok = semantics_ok(sr.ast, global);
    if (!ok) {
        fprintf(stderr, "IR: abortado por erro(s) semânticos.\n");
        st_destroy(global);
        ast_free(sr.ast);
        return 1;
    }

    // 3) IR (agora via irb_build_program)
    IrProgram *prog = irb_build_program(sr.ast);
    if (!prog) {
        fprintf(stderr, "IR: falha ao construir programa IR.\n");
        st_destroy(global);
        ast_free(sr.ast);
        return 1;
    }

    // DEBUG: Verifique quantas funções foram geradas
    // fprintf(stderr, "DEBUG IR: Número de funções no programa: %zu\n", prog->func_count);
    // for (size_t i = 0; i < prog->func_count; ++i) {
    //     fprintf(stderr, "DEBUG IR: Função %zu: %s\n", i,
    //             prog->funcs[i]->name ? prog->funcs[i]->name : "<unnamed>");
    // }

    // 4) Imprime IR
    ir_print_program(prog);

    // 5) Libera
    ir_program_free(prog);
    st_destroy(global);
    ast_free(sr.ast);
    return 0;
}
