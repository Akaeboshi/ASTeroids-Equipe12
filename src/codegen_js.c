#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ir.h"
#include "codegen_js.h"
/* -------------------------------------------------------
 *  Mapa simples de temporários JS
 * ------------------------------------------------------- */
typedef struct JsTemp {
    int temp_id;
    int declared;          /* 0 = ainda não usei let, 1 = já declarei */
    struct JsTemp *next;
} JsTemp;

static JsTemp *g_temps = NULL;

static void js_reset_temps(void) {
    JsTemp *t = g_temps;
    while (t) {
        JsTemp *n = t->next;
        free(t);
        t = n;
    }
    g_temps = NULL;
}

static JsTemp *js_get_temp(int id) {
    for (JsTemp *t = g_temps; t; t = t->next) {
        if (t->temp_id == id) return t;
    }
    JsTemp *nt = (JsTemp*)malloc(sizeof(JsTemp));
    if (!nt) {
        fprintf(stderr, "jsgen: out of memory\n");
        exit(1);
    }
    nt->temp_id = id;
    nt->declared = 0;
    nt->next = g_temps;
    g_temps = nt;
    return nt;
}

/* -------------------------------------------------------
 *  Impressão de operandos JS
 * ------------------------------------------------------- */
static void print_operand_js(IrOperand op, FILE *out) {
    switch (op.kind) {
        case IR_OPER_INT:
            fprintf(out, "%lld", op.v.i);
            break;
        case IR_OPER_FLOAT:
            fprintf(out, "%g", op.v.f);
            break;
        case IR_OPER_BOOL:
            fprintf(out, "%s", op.v.b ? "true" : "false");
            break;
        case IR_OPER_TEMP:
            fprintf(out, "t%d", op.v.temp);
            break;
        default:
            fprintf(out, "/* unsupported-op */");
            break;
    }
}


/* -------------------------------------------------------
 *  Geração de uma instrução JS
 * ------------------------------------------------------- */
static void codegen_js_instr(const IrInstr *ins, FILE *out) {
    switch (ins->op) {
        case IR_MOV: {
            if (ins->dst < 0) {
                /* Destino inválido */
                break;
            }

            JsTemp *info = js_get_temp(ins->dst);

            if (!info->declared) {
                /* Primeira vez que esse temporário aparece -> declaramos com let */
                fprintf(out, "  let t%d = ", ins->dst);
                info->declared = 1;
            } else {
                /* Já declarado antes -> apenas atribuição */
                fprintf(out, "  t%d = ", ins->dst);
            }

            print_operand_js(ins->a, out);
            fprintf(out, ";\n");
            break;
        }

        /* ============================
         * Os outros ainda serão feitos
         * ============================ */
        default:
            /* Outras instruções ainda não estão implementadas */
            /* Só comentar para lembrar que falta: */
            /* fprintf(out, "  // TODO: opcode %d ainda não suportado\n", ins->op); */
            break;
    }
}



/* -------------------------------------------------------
 *  Função JS individual
 * ------------------------------------------------------- */
static void codegen_js_func(const IrFunc *f, FILE *out) {
    if (!f) return;

    /* Reseta o mapa de temporários por função */
    js_reset_temps();

    /* Função principal (_entry) */
    if (f->name && strcmp(f->name, "_entry") == 0) {
        fprintf(out, "function _entry() {\n");

        for (size_t i = 0; i < f->code_len; ++i) {
            codegen_js_instr(&f->code[i], out);
        }

        fprintf(out, "}\n\n");
        return;
    }

    fprintf(out, "function %s(", f->name ? f->name : "fn");

    for (size_t i = 0; i < f->param_count; ++i) {
        if (i) fprintf(out, ", ");
        fprintf(out, "p%zu", i);
    }
    fprintf(out, ") {\n");
    fprintf(out, "  // TODO: gerar corpo JS desta função a partir do IR\n");
    fprintf(out, "}\n\n");
}

/* -------------------------------------------------------
 *  Programa JS
 * ------------------------------------------------------- */
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

    /* limpa tabela global de temporários */
    js_reset_temps();
}
