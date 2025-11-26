#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ir.h"
#include "codegen_js.h"

/* -------------------------------------------------------
 *  Helpers e Mapa de Variáveis
 * ------------------------------------------------------- */

/* Busca nome de variável associado a um temp; se não houver, retorna NULL */
static const char *js_name_for_temp(const IrFunc *f, int temp_id) {
    if (!f) return NULL;
    for (size_t i = 0; i < f->local_count; ++i) {
        if (f->locals[i].temp == temp_id) {
            return f->locals[i].name;
        }
    }
    return NULL;
}

/* Imprime um temporário JS (nome da variável se existir) */
static void js_print_temp(const IrFunc *f, int temp_id, FILE *out) {
    const char *vname = js_name_for_temp(f, temp_id);
    if (vname) fprintf(out, "%s", vname);
    else       fprintf(out, "t%d", temp_id);
}

/* -------------------------------------------------------
 *  Mapa simples de variáveis declaradas em JS
 *    (pra não dar "let x" duas vezes)
 * ------------------------------------------------------- */
typedef struct JsTemp {
    const char    *name;
    struct JsTemp *next;
} JsTemp;

static JsTemp *g_temps = NULL;

static int js_declared(const char *name) {
    for (JsTemp *t = g_temps; t; t = t->next) {
        if (strcmp(t->name, name) == 0) return 1;
    }
    return 0;
}

static void js_mark_declared(const char *name) {
    if (js_declared(name)) return;

    JsTemp *nt = (JsTemp*)malloc(sizeof(JsTemp));
    if (!nt) {
        fprintf(stderr, "jsgen: out of memory\n");
        exit(1);
    }

    nt->name = strdup(name);
    if (!nt->name) {
        fprintf(stderr, "jsgen: out of memory (strdup)\n");
        exit(1);
    }

    nt->next = g_temps;
    g_temps  = nt;
}


/* limpa toda a lista de variáveis declaradas */
static void js_reset_temps(void) {
    JsTemp *t = g_temps;
    while (t) {
        JsTemp *next = t->next;
        free((char*)t->name);
        free(t);
        t = next;
    }
    g_temps = NULL;
}

/* -------------------------------------------------------
 *  Impressão de operandos JS (ainda pouco usada)
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
 *    (por enquanto só IR_MOV + literais/temps)
 * ------------------------------------------------------- */
static void codegen_js_instr(const IrFunc *f, const IrInstr *ins, FILE *out) {
    switch (ins->op) {
      /* ============================
       * MOV: tN = mov ...
       * ============================ */
      case IR_MOV: {
        const char *var_name = js_name_for_temp(f, ins->dst);
        char buf[32];
        const char *vname = NULL;
        int need_decl_check = 0;

        if (var_name) {
          vname = var_name;
          need_decl_check = 1;
        } else {
            // Temporário tN: sempre podemos declarar com let
            snprintf(buf, sizeof(buf), "t%d", ins->dst);
            vname = buf;
            need_decl_check = 0;
        }

        // --- escolher se vai "let x =" ou só "x =" ---
        if (need_decl_check) {
          if (!js_declared(vname)) {
            fprintf(out, "  let %s = ", vname);
            js_mark_declared(vname);
          } else {
            fprintf(out, "  %s = ", vname);
          }
        } else {
          fprintf(out, "  let %s = ", vname);
        }

        /* imprimir o operando (se for temp, também tentar usar nome) */
        if (ins->a.kind == IR_OPER_TEMP) {
          js_print_temp(f, ins->a.v.temp, out);
        } else if (ins->a.kind == IR_OPER_INT) {
          fprintf(out, "%lld", ins->a.v.i);
        } else if (ins->a.kind == IR_OPER_FLOAT) {
          fprintf(out, "%g", ins->a.v.f);
        } else if (ins->a.kind == IR_OPER_BOOL) {
          fprintf(out, "%s", ins->a.v.b ? "true" : "false");
        } else {
          fprintf(out, "0"); /* fallback pra casos não tratados ainda */
        }

        fprintf(out, ";\n");
        break;
      }

      /* ============================
       * Aritméticos: ADD, SUB, MUL, DIV
       * ============================ */
      case IR_ADD:
      case IR_SUB:
      case IR_MUL:
      case IR_DIV: {
          const char *vname = js_name_for_temp(f, ins->dst);
          char buf[32];

          if (!vname) {
              snprintf(buf, sizeof(buf), "t%d", ins->dst);
              vname = buf;
          }

          int already = js_declared(vname);
          const char *op_str = NULL;

          switch (ins->op) {
              case IR_ADD: op_str = "+"; break;
              case IR_SUB: op_str = "-"; break;
              case IR_MUL: op_str = "*"; break;
              case IR_DIV: op_str = "/"; break;
              default:     op_str = "?"; break;
          }

          if (!already) {
              fprintf(out, "  let %s = ", vname);
              js_mark_declared(vname);
          } else {
              fprintf(out, "  %s = ", vname);
          }

          /* lado esquerdo (a) */
          if (ins->a.kind == IR_OPER_TEMP) {
              js_print_temp(f, ins->a.v.temp, out);
          } else if (ins->a.kind == IR_OPER_INT) {
              fprintf(out, "%lld", ins->a.v.i);
          } else if (ins->a.kind == IR_OPER_FLOAT) {
              fprintf(out, "%g", ins->a.v.f);
          } else if (ins->a.kind == IR_OPER_BOOL) {
              fprintf(out, "%s", ins->a.v.b ? "true" : "false");
          } else {
              fprintf(out, "0");
          }

          fprintf(out, " %s ", op_str);

          /* lado direito (b) */
          if (ins->b.kind == IR_OPER_TEMP) {
              js_print_temp(f, ins->b.v.temp, out);
          } else if (ins->b.kind == IR_OPER_INT) {
              fprintf(out, "%lld", ins->b.v.i);
          } else if (ins->b.kind == IR_OPER_FLOAT) {
              fprintf(out, "%g", ins->b.v.f);
          } else if (ins->b.kind == IR_OPER_BOOL) {
              fprintf(out, "%s", ins->b.v.b ? "true" : "false");
          } else {
              fprintf(out, "0");
          }

          fprintf(out, ";\n");
          break;
      }

      /* ============================
       * Os outros ainda serão feitos
       * ============================ */
      default:
          /* Ainda não suportado — vamos ignorar silenciosamente por enquanto
             ou você pode descomentar para debugar:
             fprintf(out, "  // TODO: opcode %d ainda não suportado\n", ins->op);
          */
          break;
    }
}

/* -------------------------------------------------------
 *  Função JS individual
 * ------------------------------------------------------- */
static void codegen_js_func(const IrFunc *f, FILE *out) {
    if (!f) return;

    /* Reseta o mapa de variáveis declaradas por função */
    js_reset_temps();

    /* Função principal (_entry) */
    if (f->name && strcmp(f->name, "_entry") == 0) {
        fprintf(out, "function _entry() {\n");

        for (size_t i = 0; i < f->code_len; ++i) {
            codegen_js_instr(f, &f->code[i], out);
        }

        fprintf(out, "}\n\n");
        return;
    }

    /* Outras funções (por enquanto só casca, sem corpo real) */
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

    /* limpa tabela global de variáveis declaradas (por segurança) */
    js_reset_temps();
}
