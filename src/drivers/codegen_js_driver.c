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
       3) IR (mesma lógica do ir_driver)
       ----------------------------- */
    IrProgram *prog = ir_program_new();     // <-- AQUI é definido "prog"
    IrFunc *entry = ir_func_begin(prog, "_entry", TY_VOID, NULL, 0);

    irb_reset_state();

    if (sr.ast->kind == ND_BLOCK) {
        for (size_t i = 0; i < sr.ast->u.as_block.count; ++i) {
            irb_emit_stmt(entry, sr.ast->u.as_block.stmts[i]);
        }
    } else {
        irb_emit_stmt(entry, sr.ast);
    }

    ir_emit_ret(entry, false, (IrOperand){ .kind = IR_OPER_NONE });
    ir_func_end(prog, entry);

    /* -----------------------------
       4) Geração JS → stdout
       (para testes & para make jsfile redirecionar)
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
