#ifndef IR_H
#define IR_H

#include <stddef.h>
#include <stdbool.h>
#include "ast_base.h" /* para TypeTag e xmalloc/xstrdup */

/* ================================
 *  Operações do IR
 * ================================ */
typedef enum {
    IR_LABEL,      /* label Lx:                         (label)              */
    IR_BR,         /* br Lx                             (label)              */
    IR_BRFALSE,    /* brfalse cond, Lx                  (a,label)            */

    IR_MOV,        /* t = mov a                         (dst,a)              */
    IR_CAST,       /* t = cast a : T                    (dst,a,cast_to)      */

    IR_ADD,        /* t = a + b                         (dst,a,b)            */
    IR_SUB,        /* t = a - b                         (dst,a,b)            */
    IR_MUL,        /* t = a * b                         (dst,a,b)            */
    IR_DIV,        /* t = a / b   (resultado em float)  (dst,a,b)            */

    IR_LT,         /* t = a <  b                        (dst,a,b) -> bool    */
    IR_LE,         /* t = a <= b                        (dst,a,b) -> bool    */
    IR_GT,         /* t = a >  b                        (dst,a,b) -> bool    */
    IR_GE,         /* t = a >= b                        (dst,a,b) -> bool    */
    IR_EQ,         /* t = a == b                        (dst,a,b) -> bool    */
    IR_NE,         /* t = a != b                        (dst,a,b) -> bool    */

    IR_CALL,       /* t = call name, args...            (dst,callee,args)    */
    IR_RET         /* ret a | ret                       (a)                  */
} IrOp;

/* ================================
 *  Operandos
 * ================================ */
typedef enum {
    IR_OPER_NONE = 0,
    IR_OPER_TEMP,      /* temporário (tN) */
    IR_OPER_INT,
    IR_OPER_FLOAT,
    IR_OPER_BOOL,
    IR_OPER_LABEL,     /* Lx */
    IR_OPER_STRING     /* string literal (para chamadas) */
} IrOperandKind;

typedef struct {
    IrOperandKind kind;
    union {
        int        temp;     /* IR_OPER_TEMP */
        long long  i;        /* IR_OPER_INT  */
        double     f;        /* IR_OPER_FLOAT*/
        int        b;        /* IR_OPER_BOOL */
        int        label;    /* IR_OPER_LABEL*/
        const char *str;     /* IR_OPER_STRING */
    } v;
} IrOperand;

/* ================================
 *  Instrução
 * ================================ */
typedef struct {
    IrOp op;              /* operação da instrução */
    int  dst;             /* destino (temporário tN), se aplicável */

    IrOperand a;          /* operando genérico A  (quando aplicável) */
    IrOperand b;          /* operando genérico B  (quando aplicável) */

    /* - Branches/labels */
    int label;            /* destino ou id do label (IR_LABEL/IR_BR/IR_BRFALSE) */

    TypeTag cast_to;      /* tipo alvo do CAST */

    const char *callee;   /* Para IR_CALL: nome da função */
    int        *args;     /* Para IR_CALL: vetor de temporários (tN) com os argumentos */
    size_t      argc;     /* Para IR_CALL: quantidade de args */
    TypeTag     ret_type; /* retorno da função chamada (para referência) */
} IrInstr;

/* ================================
 *  Função e Programa
 * ================================ */
typedef struct {
    const char *name;
    TypeTag     ret_type;

    TypeTag    *params;
    size_t      param_count;

    IrInstr    *code;
    size_t      code_len;
    size_t      code_cap;

    int temp_count;   /* próximo id de temporário a alocar */
    int label_count;  /* próximo id de label a criar */
} IrFunc;

typedef struct {
    IrFunc  **funcs;
    size_t    func_count;
    size_t    func_cap;
} IrProgram;

/* ================================
 *  API — criação/gestão
 * ================================ */
IrProgram *ir_program_new(void);
void       ir_program_free(IrProgram *p);

IrFunc    *ir_func_begin(IrProgram *p,
                         const char *name,
                         TypeTag ret_type,
                         const TypeTag *params,
                         size_t param_count);

void       ir_func_end(IrProgram *p, IrFunc *f);

/* ================================
 *  Helpers — temporários/labels
 * ================================ */
int        ir_new_temp(IrFunc *f);     /* retorna id do novo tN */
int        ir_new_label(IrFunc *f);    /* retorna id do novo Lx */

/* Builders de operando */
static inline IrOperand ir_temp(int t) {
    IrOperand op = {0};
    op.kind = IR_OPER_TEMP;
    op.v.temp = t;
    return op;
}
static inline IrOperand ir_int(long long v) {
    IrOperand op = {0};
    op.kind = IR_OPER_INT;
    op.v.i = v;
    return op;
}
static inline IrOperand ir_float(double v) {
    IrOperand op = {0};
    op.kind = IR_OPER_FLOAT;
    op.v.f = v;
    return op;
}
static inline IrOperand ir_bool(int v) {
    IrOperand op = {0};
    op.kind = IR_OPER_BOOL;
    op.v.b = v ? 1 : 0;
    return op;
}
static inline IrOperand ir_label(int L) {
    IrOperand op = {0};
    op.kind = IR_OPER_LABEL;
    op.v.label = L;
    return op;
}

/* ================================
 *  Emissão de instruções
 * ================================ */
void ir_emit_label  (IrFunc *f, int label_id);
void ir_emit_br     (IrFunc *f, int label_id);
void ir_emit_brfalse(IrFunc *f, IrOperand cond, int label_id);

int  ir_emit_mov (IrFunc *f, IrOperand a);                        /* retorna dst tN */
int  ir_emit_cast(IrFunc *f, IrOperand a, TypeTag to);            /* retorna dst tN */

int  ir_emit_bin (IrFunc *f, IrOp op, IrOperand a, IrOperand b);  /* ADD/SUB/MUL/DIV -> dst */
int  ir_emit_cmp (IrFunc *f, IrOp op, IrOperand a, IrOperand b);  /* LT/LE/GT/GE/EQ/NE -> bool dst */

int  ir_emit_call(IrFunc *f, const char *name,
                  const int *arg_temps, size_t argc,
                  TypeTag ret_type); /* retorna dst tN (ou -1 se void) */

void ir_emit_ret (IrFunc *f, bool has_value, IrOperand val);

#endif /* IR_H */
