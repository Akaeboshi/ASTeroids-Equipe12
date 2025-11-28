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
    int         depth;
    struct VarTemp *next;
} VarTemp;

static VarTemp *g_vars = NULL;
static int g_scope_depth = 0; /* profundidade atual de escopo */

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
        nv->depth = v->depth;
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
    nv->depth = g_scope_depth;
    nv->next = g_vars;
    g_vars   = nv;
}

void irb_reset_state(void) {
    vt_free_list(g_vars);
    g_vars = NULL;
    g_scope_depth = 0;
}

static void irb_enter_scope(void) {
    g_scope_depth++;
}

static void irb_leave_scope(void) {
    // Remove todas as variáveis declaradas neste nível
    VarTemp **pp = &g_vars;
    while (*pp) {
        if ((*pp)->depth == g_scope_depth) {
            VarTemp *dead = *pp;
            *pp = dead->next;
            free(dead);
        } else {
            pp = &(*pp)->next;
        }
    }
    if (g_scope_depth > 0) {
        g_scope_depth--;
    }
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
    nv->depth = g_scope_depth;
    nv->next = g_vars;
    g_vars   = nv;
    if (was_created) *was_created = true;

    (void)decl_type; /* por enquanto não emitimos init default */
    return nv->temp;
}

/* -------------------------------------------------------
 *  Constrói um programa IR completo a partir da AST
 * ------------------------------------------------------- */
IrProgram *irb_build_program(Node *ast) {
    if (!ast) return NULL;

    IrProgram *prog = ir_program_new();
    // fprintf(stderr, "DEBUG BUILDER: Iniciando construção do programa IR\n");

    // Processa a AST: se for um bloco, processa cada statement
    if (ast->kind == ND_BLOCK) {
        // fprintf(stderr, "DEBUG BUILDER: AST é um bloco com %zu statements\n", ast->u.as_block.count);

        // Primeiro: processa todas as funções
        for (size_t i = 0; i < ast->u.as_block.count; ++i) {
            Node *stmt = ast->u.as_block.stmts[i];
            if (stmt->kind == ND_FUNCTION) {
                // fprintf(stderr, "DEBUG BUILDER: Processando função %s\n", stmt->u.as_function.name);

                // Extrai informações da função
                TypeTag ret_type = stmt->u.as_function.ret_type;
                const char *name = stmt->u.as_function.name;
                size_t param_count = stmt->u.as_function.param_count;

                // fprintf(stderr, "DEBUG BUILDER: Função %s -> ret_type=%d, param_count=%zu\n",
                //        name, ret_type, param_count);

                // Prepara array de tipos dos parâmetros
                TypeTag *param_types = NULL;
                if (param_count > 0) {
                    param_types = (TypeTag*)xmalloc(sizeof(TypeTag) * param_count);
                    for (size_t j = 0; j < param_count; j++) {
                        Node *param = stmt->u.as_function.params[j];
                        param_types[j] = param->u.as_decl.type;
                        // fprintf(stderr, "DEBUG BUILDER: Parâmetro %zu: tipo=%d\n", j, param_types[j]);
                    }
                }

                // Cria a função no IR
                IrFunc *func = ir_func_begin(prog, name, ret_type, param_types, param_count);
                // fprintf(stderr, "DEBUG BUILDER: Função %s criada no IR\n", name);

                // Emite o corpo da função
                irb_reset_state();
                irb_emit_stmt(func, stmt->u.as_function.body);
                ir_func_end(prog, func);
                // fprintf(stderr, "DEBUG BUILDER: Corpo da função %s emitido\n", name);

                if (param_types) free(param_types);
            }
        }

        // Segundo: cria função _entry para código global
        // fprintf(stderr, "DEBUG BUILDER: Criando função _entry para código global\n");
        IrFunc *entry = ir_func_begin(prog, "_entry", TY_VOID, NULL, 0);
        irb_reset_state();

        // int global_stmts = 0;
        for (size_t i = 0; i < ast->u.as_block.count; ++i) {
            Node *stmt = ast->u.as_block.stmts[i];
            if (stmt->kind != ND_FUNCTION) {
                irb_emit_stmt(entry, stmt);
                // global_stmts++;
            }
        }
        // fprintf(stderr, "DEBUG BUILDER: _entry tem %d statements globais\n", global_stmts);

        ir_emit_ret(entry, false, (IrOperand){.kind = IR_OPER_NONE});
        ir_func_end(prog, entry);
        // fprintf(stderr, "DEBUG BUILDER: Função _entry criada\n");
    } else {
        // AST não é um bloco - cria apenas _entry
        // fprintf(stderr, "DEBUG BUILDER: AST não é bloco, criando apenas _entry\n");
        IrFunc *entry = ir_func_begin(prog, "_entry", TY_VOID, NULL, 0);
        irb_reset_state();
        irb_emit_stmt(entry, ast);
        ir_emit_ret(entry, false, (IrOperand){.kind = IR_OPER_NONE});
        ir_func_end(prog, entry);
    }

    // fprintf(stderr, "DEBUG BUILDER: Programa IR construído com %zu funções\n", prog->func_count);
    // for (size_t i = 0; i < prog->func_count; ++i) {
    //     fprintf(stderr, "DEBUG BUILDER: Função %zu no programa: %s\n",
    //            i, prog->funcs[i]->name ? prog->funcs[i]->name : "<unnamed>");
    // }

    return prog;
}

/* ---------------------------------------------------------
 *  Helpers de emissão
 * --------------------------------------------------------- */
__attribute__((unused))
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

            bool created = false;
            (void)vt_get(f, e->u.as_assign.name, true, TY_INT, &created);

            for (VarTemp *v = g_vars; v; v = v->next) {
                if (strcmp(v->name, e->u.as_assign.name) == 0) {
                    v->temp = rv;
                    break;
                }
            }

            ir_register_local(f, e->u.as_assign.name, rv);

            return rv;
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
    irb_enter_scope();

    for (size_t i = 0; i < blk->u.as_block.count; ++i) {
        irb_emit_stmt(f, blk->u.as_block.stmts[i]);
    }

    irb_leave_scope();
}

static void emit_if(IrFunc *f, Node *n) {
    int Lelse = ir_new_label(f);
    int Lend  = ir_new_label(f);

    int c = irb_emit_expr(f, n->u.as_if.cond);

    ir_emit_brfalse(f, ir_temp(c), Lelse);

    // THEN
    irb_emit_stmt(f, n->u.as_if.then_branch);

    ir_emit_br(f, Lend);
    ir_emit_label(f, Lelse);

    // ELSE
    if (n->u.as_if.else_branch) {
        irb_emit_stmt(f, n->u.as_if.else_branch);
    }

    ir_emit_label(f, Lend);
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
            const char *name = s->u.as_decl.name;
            TypeTag type      = s->u.as_decl.type;

            bool created = false;
            int t = vt_get(f, name, true, type, &created);
            (void)t;

            ir_register_local(f, name, t);

            if (s->u.as_decl.init) {
                int rv = irb_emit_expr(f, s->u.as_decl.init);

                for (VarTemp *v = g_vars; v; v = v->next) {
                    if (strcmp(v->name, name) == 0) {
                        v->temp = rv;
                        break;
                    }
                }

                ir_register_local(f, name, rv);
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
            /* Funções são processadas no nível do programa, não aqui */
            /* O ir_driver.c deve lidar com ND_FUNCTION */
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
    const char *name = f_node->u.as_function.name;
    TypeTag ret_type = f_node->u.as_function.ret_type;
    size_t param_count = f_node->u.as_function.param_count;

    // 1. Preencher tipos dos parâmetros
    TypeTag *param_types = NULL;
    if (param_count > 0) {
        param_types = (TypeTag*)xmalloc(sizeof(TypeTag) * param_count);
        for (size_t i = 0; i < param_count; ++i) {
            Node *param_node = f_node->u.as_function.params[i];
            param_types[i] = param_node->u.as_decl.type;
        }
    }

    VarTemp *global_snapshot = vt_clone_list(g_vars);
    irb_reset_state(); // Reseta g_vars (escopo local) e o estado do builder (implicitamente feito pelo driver)

    // O ir_func_begin (em ir.c) já inicializa temp_count=0 e label_count=0.
    IrFunc *f = ir_func_begin(p, name, ret_type, param_types, param_count);
    if (param_types) free(param_types);

    if (!f) {
        vt_restore_from(global_snapshot);
        vt_free_list(global_snapshot);
        return;
    }

    // 2. Criar escopo interno e mapear Parâmetros

    // Os parâmetros da função são t0, t1, t2, ...
    for (size_t i = 0; i < param_count; ++i) {
        Node *param_node   = f_node->u.as_function.params[i];
        const char *pname  = param_node->u.as_decl.name;
        int temp_id = (int)i;      /* t0, t1, ... */

        /* registra no map interno (g_vars) */
        vt_insert_manual(pname, temp_id);

        /* informa ao IR que existe uma variável local com esse nome/temp */
        ir_register_local(f, pname, temp_id);

        if (f->temp_count <= temp_id)
            f->temp_count = temp_id + 1;
    }

    // 4. Emitir IR do corpo da função
    irb_emit_stmt(f, f_node->u.as_function.body);

    // 5. Gerar ret implícito para funções void
    // Verifica se a última instrução é um RET.
    bool needs_implicit_ret = true;
    if (f->code_len > 0 && f->code[f->code_len - 1].op == IR_RET) {
        needs_implicit_ret = false;
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
