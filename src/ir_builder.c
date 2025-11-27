#include "ir_builder.h"
#include "ast_base.h"
#include "ast_expr.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/* ---------------------------------------------------------
 *  Map simples: variável (nome) → temporário (tN)
 * --------------------------------------------------------- */
typedef struct VarTemp {
    const char *name;
    int         temp;
    struct VarTemp *next;
} VarTemp;

static VarTemp *g_vars = NULL;

/* xstrdup/xmalloc já existem em ast_base.c */
extern void *xmalloc(size_t);
extern char *xstrdup(const char*);

/* ---------- utilitários pro map (snapshot/restore) ---------- */

/* clona (shallow) a lista encadeada de VarTemp.
 * (clonamos apenas os nós; os ponteiros de name são reutilizados) */
static VarTemp* vt_clone_list(VarTemp *head) {
    VarTemp *copy = NULL, **tail = &copy;
    for (VarTemp *v = head; v; v = v->next) {
        VarTemp *nv = (VarTemp*)xmalloc(sizeof(VarTemp));
        nv->name = v->name;
        nv->temp = v->temp;
        nv->next = NULL;
        *tail = nv;
        tail = &nv->next;
    }
    return copy;
}

static void vt_free_list(VarTemp *head) {
    while (head) {
        VarTemp *nx = head->next;
        free(head);
        head = nx;
    }
}

/* substitui g_vars por um clone de 'src' */
static void vt_restore_from(VarTemp *src) {
    /* libera g_vars atual */
    vt_free_list(g_vars);
    g_vars = vt_clone_list(src);
}

static void vt_insert_manual(const char *name, int temp_id) {
    // Verifica se já existe 
    for (VarTemp *v = g_vars; v; v = v->next) {
        if (strcmp(v->name, name) == 0) {
            v->temp = temp_id;
            return;
        }
    }

    VarTemp *nv = (VarTemp*)xmalloc(sizeof(VarTemp));
    nv->name = name; 
    nv->temp = temp_id;
    nv->next = g_vars;
    g_vars   = nv;
}

void irb_reset_state(void) {
    vt_free_list(g_vars);
    g_vars = NULL;
}

/* retorna temp atual da var; se não existir e create_if_missing=true, cria um novo tN
 * decl_type é guardado para futuras extensões (store/load, init default, etc.) */
static int vt_get(IrFunc *f, const char *name, bool create_if_missing, TypeTag decl_type, bool *was_created) {
    for (VarTemp *v = g_vars; v; v = v->next) {
        if (strcmp(v->name, name) == 0) {
            if (was_created) *was_created = false;
            return v->temp;
        }
    }
    if (!create_if_missing) {
        if (was_created) *was_created = false;
        return -1;
    }
    VarTemp *nv = (VarTemp*)xmalloc(sizeof(VarTemp));
    nv->name = name; /* NÃO duplico por padrão (nome vive na AST) */
    nv->temp = ir_new_temp(f);
    nv->next = g_vars;
    g_vars   = nv;
    if (was_created) *was_created = true;

    (void)decl_type; /* por enquanto não emitimos init default */
    return nv->temp;
}

/* ---------------------------------------------------------
 *  Helpers de emissão
 * --------------------------------------------------------- */
static inline int is_num(TypeTag t){ return t==TY_INT || t==TY_FLOAT; }

static int emit_bin(IrFunc *f, IrOp op, int lt, int rt) {
    return ir_emit_bin(f, op, ir_temp(lt), ir_temp(rt));
}
static int emit_cmp(IrFunc *f, IrOp op, int lt, int rt) {
    return ir_emit_cmp(f, op, ir_temp(lt), ir_temp(rt));
}

/* ---------------------------------------------------------
 *  Expressões
 * --------------------------------------------------------- */
static int emit_unary(IrFunc *f, Node *n);
static int emit_binary(IrFunc *f, Node *n);

int irb_emit_expr(IrFunc *f, Node *e) {
    if (!e) return -1;

    switch (e->kind) {
        case ND_INT:    return ir_emit_mov(f, ir_int(e->u.as_int.value));
        case ND_FLOAT:  return ir_emit_mov(f, ir_float(e->u.as_float.value));
        case ND_BOOL:   return ir_emit_mov(f, ir_bool(e->u.as_bool.value));

        case ND_STRING:
            /* ainda não suportamos operar strings; devolvemos um temp */
            return ir_emit_mov(f, ir_bool(0));

        case ND_IDENT: {
            int t = vt_get(f, e->u.as_ident.name, false, TY_INT, NULL);
            if (t < 0) {
                t = vt_get(f, e->u.as_ident.name, true, TY_INT, NULL);
            }
            return t;
        }

        case ND_UNARY:  return emit_unary(f, e);
        case ND_BINARY: return emit_binary(f, e);

        case ND_ASSIGN: {
            int rv = irb_emit_expr(f, e->u.as_assign.value);
            (void)vt_get(f, e->u.as_assign.name, true, TY_INT, NULL);
            (void)ir_emit_mov(f, ir_temp(rv));
            int last = f->temp_count - 1;

            for (VarTemp *v = g_vars; v; v = v->next) {
                if (strcmp(v->name, e->u.as_assign.name) == 0) { v->temp = last; break; }
            }

            ir_register_local(f, e->u.as_assign.name, last);

            return last;
        }

        case ND_EXPR:
            return irb_emit_expr(f, e->u.as_expr.expr);

        case ND_CALL: {
            size_t argc = e->u.as_call.arg_count;
            int *argv = NULL;
            if (argc > 0) {
                argv = (int*)xmalloc(sizeof(int)*argc);
                for (size_t i=0;i<argc;i++) {
                    argv[i] = irb_emit_expr(f, e->u.as_call.args[i]);
                }
            }
            /* sem info de tipos de função aqui, assumimos INT por enquanto */
            int dst = ir_emit_call(f, e->u.as_call.name, argv, argc, TY_INT);
            if (argv) free(argv);
            return (dst >= 0 ? dst : -1);
        }

        default:
            return -1;
    }
}

static int emit_unary(IrFunc *f, Node *n) {
    int v = irb_emit_expr(f, n->u.as_unary.expr);
    switch (n->u.as_unary.op) {
        case UN_NEG: {
            int z = ir_emit_mov(f, ir_int(0));
            return emit_bin(f, IR_SUB, z, v); /* 0 - v */
        }
        case UN_NOT: {
            int z = ir_emit_mov(f, ir_int(0));
            return emit_cmp(f, IR_EQ, v, z);  /* v == 0 */
        }
        default:
            return v;
    }
}

static int emit_binary(IrFunc *f, Node *n) {
    int L = irb_emit_expr(f, n->u.as_binary.left);
    int R = irb_emit_expr(f, n->u.as_binary.right);

    switch (n->u.as_binary.op) {
        case BIN_ADD: return emit_bin(f, IR_ADD, L, R);
        case BIN_SUB: return emit_bin(f, IR_SUB, L, R);
        case BIN_MUL: return emit_bin(f, IR_MUL, L, R);
        case BIN_DIV: return emit_bin(f, IR_DIV, L, R);

        case BIN_LT:  return emit_cmp(f, IR_LT,  L, R);
        case BIN_LE:  return emit_cmp(f, IR_LE,  L, R);
        case BIN_GT:  return emit_cmp(f, IR_GT,  L, R);
        case BIN_GE:  return emit_cmp(f, IR_GE,  L, R);
        case BIN_EQ:  return emit_cmp(f, IR_EQ,  L, R);
        case BIN_NEQ: return emit_cmp(f, IR_NE,  L, R);

        case BIN_AND: {
            int Lfalse = ir_new_label(f);
            int Lend   = ir_new_label(f);

            ir_emit_brfalse(f, ir_temp(L), Lfalse);

            int tR = irb_emit_expr(f, n->u.as_binary.right);
            int tres = ir_emit_mov(f, ir_temp(tR));
            ir_emit_br(f, Lend);

            ir_emit_label(f, Lfalse);
            int z = ir_emit_mov(f, ir_int(0));
            (void)z;

            ir_emit_label(f, Lend);
            return tres;
        }

        case BIN_OR: {
            int Ltrue = ir_new_label(f);
            int Lend  = ir_new_label(f);

            int Lskip = ir_new_label(f);
            ir_emit_brfalse(f, ir_temp(L), Lskip);
            ir_emit_br(f, Ltrue);
            ir_emit_label(f, Lskip);

            int tR = irb_emit_expr(f, n->u.as_binary.right);
            int tres = ir_emit_mov(f, ir_temp(tR));
            ir_emit_br(f, Lend);

            ir_emit_label(f, Ltrue);
            int one = ir_emit_mov(f, ir_int(1));
            (void)one;

            ir_emit_label(f, Lend);
            return tres;
        }

        default:
            return -1;
    }
}

/* ---------------------------------------------------------
 *  Statements
 * --------------------------------------------------------- */
static void emit_block(IrFunc *f, Node *blk) {
    for (size_t i = 0; i < blk->u.as_block.count; ++i) {
        irb_emit_stmt(f, blk->u.as_block.stmts[i]);
    }
}

static void emit_if(IrFunc *f, Node *n) {
    int Lelse = ir_new_label(f);
    int Lend  = ir_new_label(f);

    VarTemp *snapshot = vt_clone_list(g_vars);

    int c = irb_emit_expr(f, n->u.as_if.cond);

    ir_emit_brfalse(f, ir_temp(c), Lelse);
    vt_restore_from(snapshot);
    irb_emit_stmt(f, n->u.as_if.then_branch);
    ir_emit_br(f, Lend);
    ir_emit_label(f, Lelse);
    vt_restore_from(snapshot);

    if (n->u.as_if.else_branch) irb_emit_stmt(f, n->u.as_if.else_branch);
    ir_emit_label(f, Lend);
    vt_restore_from(snapshot);
    vt_free_list(snapshot);
}

static void emit_while(IrFunc *f, Node *n) {
    int Lcond = ir_new_label(f);
    int Lbody = ir_new_label(f);
    int Lend  = ir_new_label(f);

    ir_emit_label(f, Lcond);
    int c = irb_emit_expr(f, n->u.as_while.cond);
    ir_emit_brfalse(f, ir_temp(c), Lend);

    ir_emit_label(f, Lbody);
    irb_emit_stmt(f, n->u.as_while.body);
    ir_emit_br(f, Lcond);

    ir_emit_label(f, Lend);
}

void irb_emit_stmt(IrFunc *f, Node *s) {
    if (!s) return;
    switch (s->kind) {
        case ND_BLOCK:
            emit_block(f, s);
            break;

        case ND_DECL: {
            bool created = false;
            int t = vt_get(f, s->u.as_decl.name, true, s->u.as_decl.type, &created);

            ir_register_local(f, s->u.as_decl.name, t);

            if (s->u.as_decl.init) {
                int rv = irb_emit_expr(f, s->u.as_decl.init);
                (void)ir_emit_mov(f, ir_temp(rv));
                int last = f->temp_count - 1;

                for (VarTemp *v = g_vars; v; v = v->next) {
                    if (strcmp(v->name, s->u.as_decl.name) == 0) { v->temp = last; break; }
                }

                ir_register_local(f, s->u.as_decl.name, last);
            }
            break;
        }

        case ND_ASSIGN:
            (void)irb_emit_expr(f, s);
            break;

        case ND_EXPR:
            (void)irb_emit_expr(f, s->u.as_expr.expr);
            break;

        case ND_IF:
            emit_if(f, s);
            break;

        case ND_WHILE:
            emit_while(f, s);
            break;

        case ND_FOR:
            if (s->u.as_for.init) irb_emit_stmt(f, s->u.as_for.init);
            {
                int Lcond = ir_new_label(f);
                int Lbody = ir_new_label(f);
                int Lend  = ir_new_label(f);
                ir_emit_label(f, Lcond);
                int c = irb_emit_expr(f, s->u.as_for.cond);
                ir_emit_brfalse(f, ir_temp(c), Lend);
                ir_emit_label(f, Lbody);
                if (s->u.as_for.body) irb_emit_stmt(f, s->u.as_for.body);
                if (s->u.as_for.step) irb_emit_stmt(f, s->u.as_for.step);
                ir_emit_br(f, Lcond);
                ir_emit_label(f, Lend);
            }
            break;

        case ND_RETURN:
            if (s->u.as_return.expr) {
                int v = irb_emit_expr(f, s->u.as_return.expr);
                ir_emit_ret(f, true, ir_temp(v));
            } else {
                ir_emit_ret(f, false, (IrOperand){ .kind = IR_OPER_NONE });
            }
            break;

        case ND_FUNCTION:
            /* funções não são emitidas aqui por enquanto */
            break;

        default:
            break;
    }
}

/* Emite o código IR para uma única função. */
void irb_emit_func(IrProgram *p, Node *func_node) {
    if (!func_node || func_node->kind != ND_FUNCTION) return;
    
    Node *f_node = func_node;
    
    // Armazena o nome e tipos do retorno/parâmetros.
    const char *name = f_node->u.as_func.name;
    TypeTag ret_type = f_node->u.as_func.ret_type;
    
    size_t param_count = f_node->u.as_func.param_count;
    TypeTag *param_types = (TypeTag*)xmalloc(sizeof(TypeTag) * param_count);
    // Node **param_names = ... (Você precisaria dos nomes para o escopo)
    
    // **NOTA:** Como não temos a definição do struct ND_FUNCTION na AST, 
    // assumirei que a função `ir_func_begin` aceita a lista de tipos. 
    // Faremos o mapeamento dos nomes de parâmetros (se disponíveis) para temporários logo abaixo.
    
    // 1. & 3. Criar IrFunc e Resetar Estado (feita por ir_func_begin e irb_reset_state)
    
    // Salva o escopo global (útil se o builder for usado para outras coisas)
    VarTemp *global_snapshot = vt_clone_list(g_vars); 
    irb_reset_state(); // Reseta g_vars (escopo local) e o estado do builder (implicitamente feito pelo driver)

    // O ir_func_begin (em ir.c) já inicializa temp_count=0 e label_count=0.
    IrFunc *f = ir_func_begin(p, name, ret_type, param_types, param_count);
    free(param_types);
    
    if (!f) return;
    
    // 2. Criar escopo interno e mapear Parâmetros
    
    // Os parâmetros da função são t0, t1, t2, ...
    for (size_t i = 0; i < param_count; i++) {
    
       const char *param_name = f_node ->u.as_func.param_name[i];

        // O temporário para o i-ésimo parâmetro é t_i.
        vt_insert_manual(xstrdup(temp_name), (int)i); // xstrdup para manter o nome na lista g_vars
        

        vt_insert_manual(xstrdup(param_name), (int)i);
        // Para cada parâmetro, reservamos o temporário t_i. 
        // Depois disso, incrementamos temp_count para que a primeira var local seja t_param_count
        f->temp_count++; // Incrementa para refletir que t0, t1, ... t(n-1) foram usados pelos parâmetros.
    }
    
    // 4. Emitir IR do corpo da função
    irb_emit_stmt(f, f_node->u.as_func.body);

    // 5. Gerar ret implícito para funções void
    // Verifica se a última instrução é um RET.
    bool needs_implicit_ret = true;
    if (f->code_len > 0) {
        if (f->code[f->code_len - 1].op == IR_RET) {
            needs_implicit_ret = false;
        }
    }
    
    if (needs_implicit_ret && ret_type == TY_VOID) {
        ir_emit_ret(f, false, (IrOperand){ .kind = IR_OPER_NONE });
    }
    
    // 3. Restaurar estado (escopo global)
    vt_restore_from(global_snapshot);
    vt_free_list(global_snapshot);

    // Finaliza a função IR
    ir_func_end(p, f); 
}

IrProgram *irb_build_program(Node *program_node) {
    if (!program_node || program_node->kind != ND_PROGRAM) return NULL;
    
    IrProgram *p = ir_program_new(); 
    if (!p) return NULL;

    // Itera sobre todas as declarações de nível superior (que devem ser ND_FUNCTION)
    for (size_t i = 0; i < program_node->u.as_program.count; i++) {
        Node *n = program_node->u.as_program.nodes[i];
        if (n->kind == ND_FUNCTION) {
            irb_emit_func(p, n);
        } else {
            // **NOTA:** Se houver código de nível superior fora de funções, 
            // você deve criar uma função especial (ex: "_entry") e 
            // emitir esses comandos lá, chamando irb_emit_stmt. 
            // Por enquanto, ignoramos comandos de nível superior que não são funções.
        }
    }

    return p;
}