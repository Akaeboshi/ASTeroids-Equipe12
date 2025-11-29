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

/*
static int js_declared(const char *name) {
    for (JsTemp *t = g_temps; t; t = t->next) {
        if (strcmp(t->name, name) == 0) return 1;
    }
    return 0;
}
*/

/* Comente esta função pois não está sendo usada
static void js_mark_declared(const char *name) {
    if (js_declared(name)) return;

    JsTemp *nt = (JsTemp*)malloc(sizeof(JsTemp));
    nt->name = strdup(name);
    nt->next = g_temps;
    g_temps = nt;
}
*/


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

/* Helper para imprimir qualquer tipo de operando (incluindo STRING) */
static void js_print_operand(const IrFunc *f, IrOperand op, FILE *out) {
    switch (op.kind) {
        case IR_OPER_TEMP:
            js_print_temp(f, op.v.temp, out);
            break;
        case IR_OPER_INT:
            fprintf(out, "%lld", op.v.i);
            break;
        case IR_OPER_FLOAT:
            fprintf(out, "%g", op.v.f);
            break;
        case IR_OPER_BOOL:
            fprintf(out, "%s", op.v.b ? "true" : "false");
            break;
        case IR_OPER_STRING: /* <--- IMPLEMENTAÇÃO DO SUPORTE A STRING */
            /* Imprime string entre aspas duplas */
            fprintf(out, "\"%s\"", op.v.str);
            break;
        case IR_OPER_LABEL:
            fprintf(out, "/* L%d */", op.v.label);
            break;
        default:
            fprintf(out, "null");
            break;
    }
}

/* -------------------------------------------------------
 *  Geração de uma instrução JS
 * ------------------------------------------------------- */
static void codegen_js_instr(const IrFunc *f, const IrInstr *ins, FILE *out) {
    switch (ins->op) {
      
      /* ============================
       * Controle de Fluxo
       * ============================ */
      case IR_BR:
            fprintf(out, "        pc = %d; break;\n", label_to_case(ins->label));
            break;

      case IR_BRFALSE:
            fprintf(out, "        if (!");
            js_print_operand(f, ins->a, out); 
            fprintf(out, ") { pc = %d; break; }\n", label_to_case(ins->label));
            break;

      case IR_RET:
            if (ins->a.kind != IR_OPER_NONE) {
                fprintf(out, "        return ");
                js_print_operand(f, ins->a, out);
                fprintf(out, ";\n");
            } else {
                fprintf(out, "        return;\n");
            }
            break;

      case IR_LABEL:
            /* Labels já foram tratados no switch/case externo */
            break;

      /* ============================
       * Chamada de Função
       * ============================ */
      case IR_CALL: {
            if (ins->dst >= 0) {
                fprintf(out, "  ");
                js_print_temp(f, ins->dst, out);
                fprintf(out, " = ");
            } else {
                fprintf(out, "  ");
            }
            fprintf(out, "%s(", ins->callee);
            for (size_t i = 0; i < ins->argc; i++) {
                if (i) fprintf(out, ", ");
                fprintf(out, "t%d", ins->args[i]);
            }
            fprintf(out, ");\n");
            break;
      }
      
      /* ============================
       * MOV 
       * ============================ */
      case IR_MOV: {
        const char *vname = js_name_for_temp(f, ins->dst);
        if (!vname) { fprintf(out, "        t%d = ", ins->dst); }
        else        { fprintf(out, "        %s = ", vname); }

        js_print_operand(f, ins->a, out); 
        fprintf(out, ";\n");
        break;
      }

      /* ============================
       * CAST 
       * ============================ */
      case IR_CAST: {
        const char *vname = js_name_for_temp(f, ins->dst);
        if (!vname) { fprintf(out, "        t%d = ", ins->dst); }
        else        { fprintf(out, "        %s = ", vname); }

        const char *cast_func = "";
        
        /* Mapeamento baseado no ast_base.h que você enviou */
        switch (ins->cast_to) {
            case TY_INT:   
            case TY_FLOAT: 
                cast_func = "Number"; 
                break;
            case TY_BOOL:  
                cast_func = "Boolean"; 
                break;
            case TY_STRING: 
                cast_func = "String"; 
                break;
            default: 
                /* Tipos VOID ou INVALID não geram cast explícito normalmente */
                break; 
        }

        /* Gera código JS: destino = CastFunc(valor) */
        if (cast_func[0] != '\0') fprintf(out, "%s(", cast_func);
        js_print_operand(f, ins->a, out);
        if (cast_func[0] != '\0') fprintf(out, ")");
        
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
          if (!vname) { fprintf(out, "        t%d = ", ins->dst); }
          else        { fprintf(out, "        %s = ", vname); }

          const char *op_str = "?";
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
              default: break;
          }

          js_print_operand(f, ins->a, out);
          fprintf(out, " %s ", op_str);
          js_print_operand(f, ins->b, out);

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

    /* Limpa mapa anterior de labels */
    LabelMap *current = g_label_map;
    while (current) {
        LabelMap *next = current->next;
        free(current);
        current = next;
    }
    g_label_map = NULL;

    /* PRIMEIRO: Verifica se precisa de controle de fluxo (se há labels) */
    int has_labels = 0;
    for (size_t i = 0; i < f->code_len; i++) {
        if (f->code[i].op == IR_LABEL) {
            has_labels = 1;
            break;
        }
    }

    /* Gera cabeçalho da função */
    fprintf(out, "function %s(", f->name ? f->name : "fn");
    
    /* Parâmetros da função */
    for (size_t i = 0; i < f->param_count; ++i) {
        if (i) fprintf(out, ", ");
        fprintf(out, "p%zu", i);
    }
    fprintf(out, ") {\n");

    /* SEGUNDO: Declara todos os temporários usados - EVITA "let ;" */
    if (f->temp_count > 0 || has_labels) {
        fprintf(out, "  let ");
        int first_temp = 1;
        
        /* Inicializa os primeiros param_count temporários com os parâmetros */
        for (int i = 0; i < f->temp_count; i++) {
            if (!first_temp) fprintf(out, ", ");
            
            if (i < (int)f->param_count) {
                /* Parâmetros: inicializa com p0, p1, ... */
                fprintf(out, "t%d = p%d", i, i);
            } else {
                /* Outros temporários: só declara */
                fprintf(out, "t%d", i);
            }
            first_temp = 0;
        }
        
        /* Adiciona pc se houver labels */
        if (has_labels) {
            if (!first_temp) fprintf(out, ", ");
            fprintf(out, "pc = 0");
        }
        fprintf(out, ";\n");
    }
    /* Se não há temporários nem labels, não declara nada */

    /* TERCEIRO: Gera estrutura de controle se houver labels */
    if (has_labels) {
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
    } else {
        /* Sem labels - código sequencial */
        for (size_t i = 0; i < f->code_len; i++) {
            codegen_js_instr(f, &f->code[i], out);
        }
    }
    
    fprintf(out, "}\n\n");

    /* Limpa mapa de labels */
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

    /* DEBUG: Ver quantas funções temos */
    fprintf(stderr, "DEBUG: Número de funções no programa: %zu\n", prog->func_count);
    
    /* 1) Gera todas as funções (inclusive _entry) */
    for (size_t i = 0; i < prog->func_count; ++i) {
        const IrFunc *f = prog->funcs[i];
        fprintf(stderr, "DEBUG: Gerando função %zu: %s\n", i, 
                f->name ? f->name : "<unnamed>");
        codegen_js_func(f, out);
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
    }

    js_reset_temps();
}