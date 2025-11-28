#include "ir_printer.h"
#include <stdio.h>

static const char* type_str(TypeTag t) {
    switch (t) {
        case TY_INT: return "int";
        case TY_FLOAT: return "float";
        case TY_BOOL: return "bool";
        case TY_STRING: return "string";
        case TY_VOID: return "void";
        default: return "<invalid>";
    }
}

static const char* binop_str(IrOp op) {
    switch (op) {
        case IR_ADD: return "add";
        case IR_SUB: return "sub";
        case IR_MUL: return "mul";
        case IR_DIV: return "div";
        case IR_LT:  return "lt";
        case IR_LE:  return "le";
        case IR_GT:  return "gt";
        case IR_GE:  return "ge";
        case IR_EQ:  return "eq";
        case IR_NE:  return "ne";
        default:     return "??";
    }
}

static void print_operand(IrOperand o) {
    switch (o.kind) {
        case IR_OPER_NONE:  printf("_"); break;
        case IR_OPER_TEMP:  printf("t%d", o.v.temp); break;
        case IR_OPER_INT:   printf("%lld", o.v.i); break;
        case IR_OPER_FLOAT: printf("%g", o.v.f); break;
        case IR_OPER_BOOL:  printf("%s", o.v.b ? "true" : "false"); break;
        case IR_OPER_LABEL: printf("L%d", o.v.label); break;
        case IR_OPER_STRING:printf("%s", o.v.str ? o.v.str : "<null>"); break;
    }
}

static void print_instr(const IrInstr *ins) {
    switch (ins->op) {
        case IR_LABEL:
            printf("  L%d:\n", ins->label);
            break;

        case IR_BR:
            printf("  br L%d\n", ins->label);
            break;

        case IR_BRFALSE:
            printf("  brfalse "); print_operand(ins->a);
            printf(", L%d\n", ins->label);
            break;

        case IR_MOV:
            printf("  t%d = mov ", ins->dst);
            print_operand(ins->a);
            printf("\n");
            break;

        case IR_CAST:
            printf("  t%d = cast ", ins->dst);
            print_operand(ins->a);
            printf(" : %s\n", type_str(ins->cast_to));
            break;

        case IR_ADD: case IR_SUB: case IR_MUL: case IR_DIV:
            printf("  t%d = %s ", ins->dst, binop_str(ins->op));
            print_operand(ins->a); printf(", ");
            print_operand(ins->b); printf("\n");
            break;

        case IR_LT: case IR_LE: case IR_GT: case IR_GE:
        case IR_EQ: case IR_NE:
            printf("  t%d = %s ", ins->dst, binop_str(ins->op));
            print_operand(ins->a); printf(", ");
            print_operand(ins->b); printf("\n");
            break;

        case IR_CALL: {
            if (ins->dst >= 0) printf("  t%d = ", ins->dst);
            else               printf("  ");
            printf("call %s(", ins->callee ? ins->callee : "<null>");
            for (size_t i = 0; i < ins->argc; i++) {
                if (i) printf(", ");
                printf("t%d", ins->args[i]);
            }
            printf(") -> %s\n", type_str(ins->ret_type));
            break;
        }

        case IR_RET:
            if (ins->a.kind == IR_OPER_NONE) printf("  ret\n");
            else {
                printf("  ret ");
                print_operand(ins->a);
                printf("\n");
            }
            break;
    }
}

void ir_print_program(const IrProgram *p) {
    if (!p) { printf("<ir: null>\n"); return; }
    for (size_t i = 0; i < p->func_count; i++) {
        const IrFunc *f = p->funcs[i];
        printf("func %s(", f->name ? f->name : "<unnamed>");
        for (size_t k = 0; k < f->param_count; k++) {
            if (k) printf(", ");
            printf("%s", type_str(f->params[k]));
        }
        printf(") -> %s {\n", type_str(f->ret_type));


        for (size_t k = 0; k < f->local_count; k++) {
            printf("  .local %s -> t%d\n", f->locals[k].name, f->locals[k].temp);
        }

        for (size_t j = 0; j < f->code_len; j++) {
            print_instr(&f->code[j]);
        }

        printf("}\n");
    }
}
