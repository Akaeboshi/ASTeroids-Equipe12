#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ir.h"
#include "codegen_js.h"

/* -------------------------------------------------------
 *  Mapa de Labels para Cases
 * ------------------------------------------------------- */
typedef struct LabelMap {
    int label_id;
    int case_index;
    struct LabelMap *next;
} LabelMap;

static LabelMap *g_label_map = NULL;

/* Busca caso correspondente a um label */
static int label_to_case(int label_id) {
    for (LabelMap *m = g_label_map; m; m = m->next) {
        if (m->label_id == label_id) {
            return m->case_index;
        }
    }
    return -1;
}

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
    nt->name = strdup(name);
    nt->next = g_temps;
    g_temps = nt;
}

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
 *  Geração de uma instrução JS
 * ------------------------------------------------------- */
static void codegen_js_instr(const IrFunc *f, const IrInstr *ins, FILE *out) {
    switch (ins->op) {
      
      /* ============================
       * Controle de Fluxo: BR, BRFALSE, RET
       * ============================ */
      case IR_BR:
            fprintf(out, "        pc = %d; break;\n", label_to_case(ins->label));
            break;

      case IR_BRFALSE:
            fprintf(out, "        if (!");
            js_print_temp(f, ins->a.v.temp, out);
            fprintf(out, ") { pc = %d; break; }\n", label_to_case(ins->label));
            break;

      case IR_RET:
            if (ins->a.kind != IR_OPER_NONE) {
                fprintf(out, "        return ");
                js_print_temp(f, ins->a.v.temp, out);
                fprintf(out, ";\n");
            } else {
                fprintf(out, "        return;\n");
            }
            break;

      case IR_LABEL:
            /* Labels são tratados na estrutura de controle */
            break;
      
      /* ============================
       * MOV: tN = mov ...
       * ============================ */
      case IR_MOV: {
        const char *var_name = js_name_for_temp(f, ins->dst);
        char buf[32];
        const char *vname = NULL;

        if (var_name) {
          vname = var_name;
        } else {
            snprintf(buf, sizeof(buf), "t%d", ins->dst);
            vname = buf;
        }

        fprintf(out, "        %s = ", vname);

        /* imprimir o operando */
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

        fprintf(out, ";\n");
        break;
      }

      /* ============================
       * Aritméticos e Comparações
       * ============================ */
      case IR_ADD:
      case IR_SUB:
      case IR_MUL:
      case IR_DIV:
      case IR_LT:
      case IR_LE:
      case IR_GT:
      case IR_GE:
      case IR_EQ:
      case IR_NE: {
          const char *vname = js_name_for_temp(f, ins->dst);
          char buf[32];

          if (!vname) {
              snprintf(buf, sizeof(buf), "t%d", ins->dst);
              vname = buf;
          }

          const char *op_str = NULL;
          switch (ins->op) {
              case IR_ADD: op_str = "+"; break;
              case IR_SUB: op_str = "-"; break;
              case IR_MUL: op_str = "*"; break;
              case IR_DIV: op_str = "/"; break;
              case IR_LT: op_str = "<"; break;
              case IR_LE: op_str = "<="; break;
              case IR_GT: op_str = ">"; break;
              case IR_GE: op_str = ">="; break;
              case IR_EQ: op_str = "==="; break; 
              case IR_NE: op_str = "!=="; break;
              default:     op_str = "?"; break;
          }

          fprintf(out, "        %s = ", vname);

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
      
      default:
          break;
    }
}

/* -------------------------------------------------------
 *  Função JS individual com Controle de Fluxo
 * ------------------------------------------------------- */
static void codegen_js_func(const IrFunc *f, FILE *out) {
    if (!f) return;

    js_reset_temps();

    /* Limpa mapa anterior */
    LabelMap *current = g_label_map;
    while (current) {
        LabelMap *next = current->next;
        free(current);
        current = next;
    }
    g_label_map = NULL;

    if (f->name && strcmp(f->name, "_entry") == 0) {
        fprintf(out, "function _entry() {\n");
        
        /* Declara todos os temporários usados */
        fprintf(out, "  let ");
        int first_temp = 1;
        for (int i = 0; i < f->temp_count; i++) {
            if (!first_temp) fprintf(out, ", ");
            fprintf(out, "t%d", i);
            first_temp = 0;
        }
        fprintf(out, ", pc = 0;\n");

        /* Constrói mapa de labels -> casos */
        int case_count = 0;
        
        /* Case 0 é sempre o início */
        case_count++;
        
        /* Conta labels para determinar quantos casos precisamos */
        for (size_t i = 0; i < f->code_len; i++) {
            if (f->code[i].op == IR_LABEL) {
                LabelMap *new_map = malloc(sizeof(LabelMap));
                new_map->label_id = f->code[i].label;
                new_map->case_index = case_count++;
                new_map->next = g_label_map;
                g_label_map = new_map;
            }
        }

        /* Gera estrutura de controle */
        fprintf(out, "  while (true) {\n");
        fprintf(out, "    switch (pc) {\n");
        
        /* Gera todos os casos */
        int current_case = 0;
        int has_instructions = 0;
        
        /* Case 0 - início */
        fprintf(out, "      case %d:\n", current_case);
        
        for (size_t i = 0; i < f->code_len; i++) {
            IrInstr ins = f->code[i];
            
            if (ins.op == IR_LABEL) {
                if (has_instructions) {
                    fprintf(out, "        break;\n");
                }
                current_case = label_to_case(ins.label);
                fprintf(out, "      case %d: // L%d\n", current_case, ins.label);
                has_instructions = 0;
            } else {
                codegen_js_instr(f, &ins, out);
                has_instructions = 1;
            }
        }
        
        if (has_instructions) {
            fprintf(out, "        break;\n");
        }
        
        fprintf(out, "    }\n");
        fprintf(out, "    break;\n");
        fprintf(out, "  }\n");
        fprintf(out, "}\n\n");

        /* Limpa mapa de labels */
        current = g_label_map;
        while (current) {
            LabelMap *next = current->next;
            free(current);
            current = next;
        }
        g_label_map = NULL;
        return;
    }

    /* Outras funções */
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

    /* Gera todas as funções */
    for (size_t i = 0; i < prog->func_count; ++i) {
        codegen_js_func(prog->funcs[i], out);
    }

    /* Chama _entry se existir */
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
    }

    js_reset_temps();
}