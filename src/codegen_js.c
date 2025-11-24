#include "codegen_js.h"
#include "ir.h"
#include <stdio.h>
#include <string.h>

static void codegen_js_func(const IrFunc *f, FILE *out) {
    if (!f) return;

    /* Função principal (_entry) */
    if (f->name && strcmp(f->name, "_entry") == 0) {
        fprintf(out, "function _entry() {\n");
        fprintf(out, "  // TODO: gerar código a partir do IR desta função\n");
        fprintf(out, "  // (labels, branches, temporários, etc.)\n");
        fprintf(out, "}\n\n");
        return;
    }

    /* Outras funções */
    fprintf(out, "function %s(", f->name ? f->name : "fn") ;

    for (size_t i = 0; i < f->param_count; ++i) {
        if (i) fprintf(out, ", ");
        fprintf(out, "p%zu", i);
    }
    fprintf(out, ") {\n");
    fprintf(out, "  // TODO: gerar corpo JS desta função a partir do IR\n");
    fprintf(out, "}\n\n");
}

void codegen_js_program(const IrProgram *prog, FILE *out) {
    if (!prog) {
        fprintf(out, "// <jsgen: programa nulo>\n");
        return;
    }
    if (!out) out = stdout;

    fprintf(out, "// Código gerado automaticamente a partir do IR\n\n");

    /* 1) Gera todas as funções (inclusive _entry) */
    for (size_t i = 0; i < prog->func_count; ++i) {
        codegen_js_func(prog->funcs[i], out);
    }

    /* 2) Se existir _entry, chamamos no final */
    int has_entry = 0;
    for (size_t i = 0; i < prog->func_count; ++i) {
        const IrFunc *f = prog->funcs[i];
        if (f->name && strcmp(f->name, "_entry") == 0) {
            has_entry = 1;
            break;
        }
    }

    if (has_entry) {
        fprintf(out, "_entry();\n");
    } else {
        fprintf(out, "// TODO: não foi encontrada função _entry()\n");
    }
}
