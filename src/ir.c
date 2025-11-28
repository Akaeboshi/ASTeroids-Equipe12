#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ===== Internals ===== */

static void *ir_xrealloc(void *p, size_t n) {
    void *q = realloc(p, n);
    if (!q) { fprintf(stderr, "error: realloc failed\n"); exit(1); }
    return q;
}

static void ir_func_grow(IrFunc *f) {
    if (f->code_len + 1 > f->code_cap) {
        f->code_cap = f->code_cap ? f->code_cap * 2 : 16;
        f->code = ir_xrealloc(f->code, sizeof(IrInstr) * f->code_cap);
    }
}

static void ir_push(IrFunc *f, const IrInstr *ins) {
    ir_func_grow(f);
    f->code[f->code_len++] = *ins;
}

/* ===== Programa ===== */


// Criação de um novo programa IR
IrProgram *ir_program_new(void) {
    IrProgram *p = (IrProgram*)xmalloc(sizeof(IrProgram));
    p->funcs = NULL;
    p->func_count = 0;
    p->func_cap = 0;
    return p;
}

// Liberação de um programa IR
void ir_program_free(IrProgram *p) {
    if (!p) return;
    for (size_t i = 0; i < p->func_count; ++i) {
        IrFunc *f = p->funcs[i];
        if (!f) continue;

        /* libera code */
        for (size_t k = 0; k < f->code_len; ++k) {
            IrInstr *ins = &f->code[k];
            if (ins->op == IR_CALL) {
                /* vetor de args */
                if (ins->args) free(ins->args);
            }
        }
        free(f->code);

        if (f->params) free(f->params);

        if (f->locals) free(f->locals);

        free(f);
    }
    free(p->funcs);
    free(p);
}

/* ===== Funções ===== */

// Garante espaço para mais uma função no programa
static void ir_program_grow_funcs(IrProgram *p) {
    if (p->func_count + 1 > p->func_cap) {
        p->func_cap = p->func_cap ? p->func_cap * 2 : 8;
        p->funcs = ir_xrealloc(p->funcs, sizeof(IrFunc*) * p->func_cap);
    }
}


// Inicia a criação de uma nova função no programa
IrFunc *ir_func_begin(IrProgram *p,
                      const char *name,
                      TypeTag ret_type,
                      const TypeTag *params,
                      size_t param_count)
{
    IrFunc *f = (IrFunc*)xmalloc(sizeof(IrFunc));
    f->name        = name;
    f->ret_type    = ret_type;
    f->code        = NULL;
    f->code_len    = 0;
    f->code_cap    = 0;
    f->temp_count  = 0;
    f->label_count = 0;
    f->locals      = NULL;
    f->local_count = 0;
    f->local_cap   = 0;

    if (param_count > 0) {
        f->params = (TypeTag*)xmalloc(sizeof(TypeTag)*param_count);
        memcpy(f->params, params, sizeof(TypeTag)*param_count);
        f->param_count = param_count;
    } else {
        f->params = NULL;
        f->param_count = 0;
    }

    ir_program_grow_funcs(p);
    p->funcs[p->func_count++] = f;
    return f;
}

// Finaliza a criação de uma função no programa
void ir_func_end(IrProgram *p, IrFunc *f) {
    (void)p;
    (void)f;
}

/* ===== Temporários e Labels ===== */

// Gera um novo temporário tN
int ir_new_temp(IrFunc *f) {
    return f->temp_count++; /* t0, t1, t2, ... */
}

// Gera um novo label Lx
int ir_new_label(IrFunc *f) {
    return f->label_count++; /* L0, L1, L2, ... */
}

/* ===== Emissão de instruções ===== */

// Emite uma instrução de label
void ir_emit_label(IrFunc *f, int label_id) {
    IrInstr ins = {0};
    ins.op    = IR_LABEL;
    ins.label = label_id;
    ins.dst   = -1;
    ins.a.kind = IR_OPER_NONE;
    ins.b.kind = IR_OPER_NONE;
    ir_push(f, &ins);
}

// Emite uma instrução de branch incondicional
void ir_emit_br(IrFunc *f, int label_id) {
    IrInstr ins = {0};
    ins.op    = IR_BR;
    ins.label = label_id;
    ins.dst   = -1;
    ins.a.kind = IR_OPER_NONE;
    ins.b.kind = IR_OPER_NONE;
    ir_push(f, &ins);
}

// Emite uma instrução de branch condicional
void ir_emit_brfalse(IrFunc *f, IrOperand cond, int label_id) {
    IrInstr ins = {0};
    ins.op    = IR_BRFALSE;
    ins.a     = cond;
    ins.label = label_id;
    ins.dst   = -1;
    ins.b.kind = IR_OPER_NONE;
    ir_push(f, &ins);
}


// Emite uma instrução de movimentação
int ir_emit_mov(IrFunc *f, IrOperand a) {
    int dst = ir_new_temp(f);
    IrInstr ins = {0};
    ins.op  = IR_MOV;
    ins.dst = dst;
    ins.a   = a;
    ins.b.kind = IR_OPER_NONE;
    ir_push(f, &ins);
    return dst;
}

// Emite uma instrução de cast
int ir_emit_cast(IrFunc *f, IrOperand a, TypeTag to) {
    int dst = ir_new_temp(f);
    IrInstr ins = {0};
    ins.op     = IR_CAST;
    ins.dst    = dst;
    ins.a      = a;
    ins.cast_to = to;
    ins.b.kind = IR_OPER_NONE;
    ir_push(f, &ins);
    return dst;
}

// Emite uma instrução binária
static int ir_emit_bin_like(IrFunc *f, IrOp op, IrOperand a, IrOperand b) {
    int dst = ir_new_temp(f);
    IrInstr ins = {0};
    ins.op  = op;
    ins.dst = dst;
    ins.a   = a;
    ins.b   = b;
    ir_push(f, &ins);
    return dst;
}

// Emite uma instrução binária
int ir_emit_bin(IrFunc *f, IrOp op, IrOperand a, IrOperand b) {
    /* deve ser IR_ADD/IR_SUB/IR_MUL/IR_DIV */
    return ir_emit_bin_like(f, op, a, b);
}


// Emite uma instrução de comparação
int ir_emit_cmp(IrFunc *f, IrOp op, IrOperand a, IrOperand b) {
    /* deve ser IR_LT/IR_LE/IR_GT/IR_GE/IR_EQ/IR_NE */
    return ir_emit_bin_like(f, op, a, b);
}

// Emite uma instrução de chamada de função
int ir_emit_call(IrFunc *f, const char *name,
                 const int *arg_temps, size_t argc,
                 TypeTag ret_type)
{
    IrInstr ins = {0};
    ins.op       = IR_CALL;
    ins.callee   = name;
    ins.argc     = argc;
    ins.ret_type = ret_type;

    if (argc > 0) {
        ins.args = (int*)xmalloc(sizeof(int)*argc);
        for (size_t i=0;i<argc;i++) ins.args[i] = arg_temps[i];
    }

    if (ret_type != TY_VOID) {
        ins.dst = ir_new_temp(f);
    } else {
        ins.dst = -1;
    }

    ins.a.kind = IR_OPER_NONE;
    ins.b.kind = IR_OPER_NONE;
    ir_push(f, &ins);
    return ins.dst;
}

// Emite uma instrução de retorno
void ir_emit_ret(IrFunc *f, bool has_value, IrOperand val) {
    IrInstr ins = {0};
    ins.op = IR_RET;
    if (has_value) ins.a = val; else ins.a.kind = IR_OPER_NONE;
    ins.dst = -1;
    ins.b.kind = IR_OPER_NONE;
    ir_push(f, &ins);
}

/* Aumenta vetor de variáveis locais */
static void ir_func_grow_locals(IrFunc *f) {
    if (f->local_count + 1 > f->local_cap) {
        f->local_cap = f->local_cap ? f->local_cap * 2 : 8;
        f->locals = (IrLocalVar*)ir_xrealloc(f->locals, sizeof(IrLocalVar) * f->local_cap);
    }
}

/* Registra uma variável local */
void ir_register_local(IrFunc *f, const char *name, int temp) {
    if (!f || !name) return;

    /* Não sobrescrevemos entradas antigas: cada temp mantém o nome associado. */
    for (size_t i = 0; i < f->local_count; ++i) {
        if (f->locals[i].temp == temp && strcmp(f->locals[i].name, name) == 0) {
            return; /* já registramos este par nome/temp */
        }
    }

    ir_func_grow_locals(f);
    f->locals[f->local_count].name = name;
    f->locals[f->local_count].temp = temp;
    f->local_count++;
}
