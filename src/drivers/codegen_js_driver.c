#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ast.h"
#include "ir.h"
#include "ir_builder.h"
#include "codegen_js.h"
#include "syntax_analyzer.h"
#include "semantic_analyzer.h"

extern int g_parse_errors;

/* Detecta leitura de stdin */
static bool read_from_stdin(const char *arg) {
    if (!arg) return true;
    return (strcmp(arg, "-") == 0 || strcmp(arg, "--") == 0);
}

int main(int argc, char **argv) {
    const char *path = NULL;

    if (argc > 1 && !read_from_stdin(argv[1])) {
        path = argv[1];
    }

    /* -----------------------------
       1) Sintaxe
       ----------------------------- */
    SyntaxResult sr = syntax_parse_path(path);
    if (!sr.parse_ok || sr.ast == NULL || g_parse_errors > 0) {
        fprintf(stderr, "JS: abortado por erro(s) sintáticos.\n");
        return 1;
    }

    /* -----------------------------
       2) Semântica
       ----------------------------- */
    SymbolTable *global = st_create();
    int ok = semantics_ok(sr.ast, global);
    if (!ok) {
        fprintf(stderr, "JS: abortado por erro(s) semânticos.\n");
        st_destroy(global);
        ast_free(sr.ast);
        return 1;
    }

    /* -----------------------------
       3) IR (usando irb_build_program - MESMO que irgen)
       ----------------------------- */
    IrProgram *prog = irb_build_program(sr.ast);
    if (!prog) {
        fprintf(stderr, "JS: falha ao construir programa IR.\n");
        st_destroy(global);
        ast_free(sr.ast);
        return 1;
    }

    // DEBUG: Verifique quantas funções foram recebidas
    // fprintf(stderr, "DEBUG JS: Número de funções no programa: %zu\n", prog->func_count);
    // for (size_t i = 0; i < prog->func_count; ++i) {
    //     fprintf(stderr, "DEBUG JS: Função %zu: %s\n", i,
    //             prog->funcs[i]->name ? prog->funcs[i]->name : "<unnamed>");
    // }

    /* -----------------------------
       4) Geração JS → stdout
       ----------------------------- */
    codegen_js_program(prog, stdout);

    /* -----------------------------
       5) Libera tudo
       ----------------------------- */
    ir_program_free(prog);
    st_destroy(global);
    ast_free(sr.ast);

    return 0;
}
