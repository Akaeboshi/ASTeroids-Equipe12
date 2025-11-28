#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ir.h"
#include "codegen_js.h"

/* -------------------------------------------------------
 *  Mapa de Labels para Cases (modo com controle de fluxo)
 * ------------------------------------------------------- */
typedef struct LabelMap {
    int label_id;
    int case_index;
    struct LabelMap *next;
} LabelMap;

static LabelMap *g_label_map = NULL;

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
 *  (usado no modo SEQUENCIAL, sem labels)
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
 *  Geração de uma instrução JS
 *
 *  seq_mode = 1 → função sequencial (sem labels):
 *      - usamos "let" na primeira vez que uma variável aparece.
 *  seq_mode = 0 → função com labels (máquina de estados):
 *      - nunca usamos "let" dentro do switch/cases.
 * ------------------------------------------------------- */
static void codegen_js_instr(const IrFunc *f, const IrInstr *ins, FILE *out, int seq_mode) {
    switch (ins->op) {

      /* ============================
       * Controle de Fluxo (modo com labels)
       * ============================ */
      case IR_BR:
            if (!seq_mode) {
                fprintf(out, "        pc = %d; break;\n", label_to_case(ins->label));
            }
            break;

      case IR_BRFALSE:
            if (!seq_mode) {
                fprintf(out, "        if (!");
                js_print_temp(f, ins->a.v.temp, out);
                fprintf(out, ") { pc = %d; break; }\n", label_to_case(ins->label));
            }
            break;

      case IR_RET:
            if (seq_mode) {
                /* No modo sequencial, não queremos 'return;' vazio em _entry */
                if (f->name && strcmp(f->name, "_entry") == 0 &&
                    ins->a.kind == IR_OPER_NONE) {
                    /* não imprime nada */
                    break;
                }

                if (ins->a.kind != IR_OPER_NONE) {
                    fprintf(out, "  return ");
                    js_print_temp(f, ins->a.v.temp, out);
                    fprintf(out, ";\n");
                } else {
                    fprintf(out, "  return;\n");
                }
            } else {
                /* modo com labels: mantém indentação antiga */
                if (ins->a.kind != IR_OPER_NONE) {
                    fprintf(out, "        return ");
                    js_print_temp(f, ins->a.v.temp, out);
                    fprintf(out, ";\n");
                } else {
                    fprintf(out, "        return;\n");
                }
            }
            break;

      case IR_LABEL:
            /* Labels só são relevantes no modo com labels (máquina de estados),
               mas a 'estrutura' é gerada em codegen_js_func. Aqui, nada. */
            break;

      /* ============================
       * Chamada de Função: IR_CALL
       * ============================ */
      case IR_CALL: {
            const char *dst_name = NULL;
            char buf[32];

            if (ins->dst >= 0) {
                dst_name = js_name_for_temp(f, ins->dst);
                if (!dst_name) {
                    snprintf(buf, sizeof(buf), "t%d", ins->dst);
                    dst_name = buf;
                }
            }

            if (seq_mode) {
                if (dst_name) {
                    if (!js_declared(dst_name)) {
                        fprintf(out, "  let %s = ", dst_name);
                        js_mark_declared(dst_name);
                    } else {
                        fprintf(out, "  %s = ", dst_name);
                    }
                } else {
                    fprintf(out, "  ");
                }
            } else {
                if (dst_name) {
                    fprintf(out, "        ");
                    js_print_temp(f, ins->dst, out);
                    fprintf(out, " = ");
                } else {
                    fprintf(out, "        ");
                }
            }

            /* Nome da função */
            fprintf(out, "%s(", ins->callee ? ins->callee : "fn");

            /* Argumentos */
            for (size_t i = 0; i < ins->argc; i++) {
                if (i) fprintf(out, ", ");
                fprintf(out, "t%d", ins->args[i]);
            }

            fprintf(out, ");\n");
            break;
      }

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

        if (seq_mode) {
            /* modo simples: usamos let na primeira vez */
            if (!js_declared(vname)) {
                fprintf(out, "  let %s = ", vname);
                js_mark_declared(vname);
            } else {
                fprintf(out, "  %s = ", vname);
            }
        } else {
            /* modo com labels: já declaramos tudo no topo */
            fprintf(out, "        %s = ", vname);
        }

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

          if (seq_mode) {
              if (!js_declared(vname)) {
                  fprintf(out, "  let %s = ", vname);
                  js_mark_declared(vname);
              } else {
                  fprintf(out, "  %s = ", vname);
              }
          } else {
              fprintf(out, "        %s = ", vname);
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

      default:
          break;
    }
}

/* -------------------------------------------------------
 *  Função JS individual
 * ------------------------------------------------------- */
static void codegen_js_func(const IrFunc *f, FILE *out) {
    if (!f) return;

    js_reset_temps();

    /* Limpa mapa anterior de labels */
    LabelMap *current = g_label_map;
    while (current) {
        LabelMap *next = current->next;
        free(current);
        current = next;
    }
    g_label_map = NULL;

    /* Verifica se essa função usa labels */
    int has_labels = 0;
    for (size_t i = 0; i < f->code_len; i++) {
        if (f->code[i].op == IR_LABEL) {
            has_labels = 1;
            break;
        }
    }

    /* Cabeçalho da função */
    fprintf(out, "function %s(", f->name ? f->name : "fn");

    for (size_t i = 0; i < f->param_count; ++i) {
        if (i) fprintf(out, ", ");
        fprintf(out, "p%zu", i);
    }
    fprintf(out, ") {\n");

    if (has_labels) {
        /* ============================
         *  MODO COM LABELS: máquina de estados
         * ============================ */

        /* Declara temporários e pc no topo */
        if (f->temp_count > 0 || has_labels) {
            fprintf(out, "  let ");
            int first_temp = 1;

            for (int i = 0; i < f->temp_count; i++) {
                if (!first_temp) fprintf(out, ", ");
                if (i < (int)f->param_count) {
                    fprintf(out, "t%d = p%d", i, i);
                } else {
                    fprintf(out, "t%d", i);
                }
                first_temp = 0;
            }

            if (has_labels) {
                if (!first_temp) fprintf(out, ", ");
                fprintf(out, "pc = 0");
            }
            fprintf(out, ";\n");
        }

        /* Constrói mapa de labels -> cases */
        int case_count = 0;
        case_count++; /* case 0 = início */

        for (size_t i = 0; i < f->code_len; i++) {
            if (f->code[i].op == IR_LABEL) {
                LabelMap *new_map = (LabelMap*)malloc(sizeof(LabelMap));
                new_map->label_id = f->code[i].label;
                new_map->case_index = case_count++;
                new_map->next = g_label_map;
                g_label_map = new_map;
            }
        }

        fprintf(out, "  while (true) {\n");
        fprintf(out, "    switch (pc) {\n");

        int current_case = 0;
        int has_instructions = 0;
        int last_was_br_or_ret = 0;

        fprintf(out, "      case %d:\n", current_case);

        for (size_t i = 0; i < f->code_len; i++) {
            IrInstr ins = f->code[i];

            if (ins.op == IR_LABEL) {
                if (has_instructions) {
                    int next_case = label_to_case(ins.label);

                    if (!last_was_br_or_ret) {
                        // fall-through para o próximo bloco (próxima label)
                        fprintf(out, "        pc = %d; break;\n", next_case);
                    } else {
                        // bloco anterior já tinha br/ret → só fecha o case
                        fprintf(out, "        break;\n");
                    }
                }

                current_case = label_to_case(ins.label);
                fprintf(out, "      case %d: // L%d\n", current_case, ins.label);
                has_instructions = 0;
                last_was_br_or_ret = 0;
            } else {
                codegen_js_instr(f, &ins, out, 0);

                has_instructions = 1;

                if (ins.op == IR_BR || ins.op == IR_RET) {
                    last_was_br_or_ret = 1;
                } else {
                    last_was_br_or_ret = 0;
                }
            }
        }

        // Fecha o último bloco
        if (has_instructions) {
            // último bloco: se não terminou com br/ret, só dá um break;
            fprintf(out, "        break;\n");
        }

        fprintf(out, "    }\n");
        fprintf(out, "  }\n");

    } else {
        /* ============================
         *  MODO SEQUENCIAL: sem labels
         * ============================ */
        for (size_t i = 0; i < f->code_len; i++) {
            codegen_js_instr(f, &f->code[i], out, 1 /* seq_mode = 1 */);
        }
    }

    fprintf(out, "}\n\n");

    /* limpa mapa de labels */
    current = g_label_map;
    while (current) {
        LabelMap *next = current->next;
        free(current);
        current = next;
    }
    g_label_map = NULL;
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

    for (size_t i = 0; i < prog->func_count; ++i) {
        codegen_js_func(prog->funcs[i], out);
    }

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
