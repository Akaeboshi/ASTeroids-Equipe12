#include "semantic_analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* =========================================================
 * Protótipos (para resolver ordem de chamadas internas)
 * ========================================================= */
static void   check_stmt  (Node *n, int *errors);
static void   check_assign(Node *as, int *errors);
static TypeTag infer      (Node *n, int *errors);



/* =========================================================
 * Estado de escopos (pilha encadeada)
 *    - push_scope_new    : cria escopo novo "filho" do atual
 *    - push_scope_with   : empilha escopo usando uma tabela já existente
 *    - pop_scope         : desempilha (e destrói se 'owns' == 1)
 * ========================================================= */
typedef struct Scope {
    SymbolTable *tab;
    struct Scope *parent;
    int owns; /* 1: destruir tab ao sair; 0: não destruir (tabela injetada) */
} Scope;

static Scope *g_scope = NULL;

static void push_scope_new(void) {
    Scope *s = (Scope*)malloc(sizeof(Scope));
    s->tab = st_create();
    s->tab->parent = g_scope ? g_scope->tab : NULL;
    s->parent = g_scope;
    s->owns = 1;
    g_scope = s;
}

static void push_scope_with(SymbolTable *tab) {
    Scope *s = (Scope*)malloc(sizeof(Scope));
    s->tab = tab;
    s->parent = g_scope;
    s->owns = 0;
    g_scope = s;
}

static void pop_scope(void) {
    if (!g_scope) return;
    Scope *old = g_scope;
    g_scope = g_scope->parent;
    if (old->owns) st_destroy(old->tab);
    free(old);
}

/* =========================================================
 * Helpers de escopo/lookup
 *    - exists_local : verifica redeclaração apenas no escopo atual
 *    - lookup_type  : procura tipo de IDENT subindo a cadeia de escopos
 * ========================================================= */
static bool exists_local(const char *name) {
    if (!g_scope || !g_scope->tab) return false;
    SymbolTable *t = g_scope->tab;
    SymbolTable *saved_parent = t->parent;
    t->parent = NULL;
    bool found = false;
    (void)st_lookup_type_recursive(t, name, &found);
    t->parent = saved_parent;
    return found;
}

static int lookup_type(const char *name, TypeTag *out) {
    for (Scope *s = g_scope; s; s = s->parent) {
        bool ok = false;
        TypeTag t = st_lookup_type_recursive(s->tab, name, &ok);
        if (ok) { if (out) *out = t; return 1; }
    }
    return 0;
}

/* =========================================================
 * Pilha de tipo de retorno de função
 * ========================================================= */
#define MAX_FUN_STACK 64
static TypeTag g_fun_ret_stack[MAX_FUN_STACK];
static int     g_fun_sp = 0;

static void push_fun_ret(TypeTag t) { if (g_fun_sp < MAX_FUN_STACK) g_fun_ret_stack[g_fun_sp++] = t; }
static void pop_fun_ret(void) { if (g_fun_sp > 0) g_fun_sp--; }
static TypeTag current_fun_ret(void) { return (g_fun_sp>0)? g_fun_ret_stack[g_fun_sp-1] : TY_VOID; }

/* =========================================================
 * Registro e lookup de funções globais
 * ========================================================= */
typedef struct FunSig {
    const char *name;
    TypeTag     ret;
    TypeTag    *params;
    size_t      param_count;
    struct FunSig *next;
} FunSig;

static FunSig *g_funs = NULL;

/**
 * Procura uma função registrada por nome.
 */
static const FunSig* find_fun(const char *name) {
    for (const FunSig *f = g_funs; f; f = f->next)
        if (strcmp(f->name, name) == 0)
            return f;
    return NULL;
}

/**
 * Registra uma função global.
 * Copia tipos e armazena em lista encadeada global.
 */
static void register_fun(const char *name, TypeTag ret, Node **param_nodes, size_t n) {
    FunSig *f = (FunSig*)malloc(sizeof(FunSig));
    f->name  = xstrdup(name);
    f->ret   = ret;
    f->param_count = n;
    if (n > 0) {
        f->params = (TypeTag*)malloc(sizeof(TypeTag) * n);
        for (size_t i = 0; i < n; i++) {
            Node *pd = param_nodes[i]; /* ND_DECL */
            f->params[i] = pd->u.as_decl.type;
        }
    } else {
        f->params = NULL;
    }
    f->next = g_funs;
    g_funs = f;
}

/* =========================================================
 * Helpers de tipo e inferência de expressões
 *    - is_numeric
 *    - infer_* (unário/binário)
 *    - infer (despacha por nó)
 * ========================================================= */
static inline int is_numeric(TypeTag t) { return t == TY_INT || t == TY_FLOAT; }

static TypeTag infer_unary(Node *n, int *errors) {
    TypeTag t = infer(n->u.as_unary.expr, errors);
    switch (n->u.as_unary.op) {
        case UN_NEG: return is_numeric(t) ? t : TY_INVALID;
        case UN_NOT: return (t == TY_BOOL) ? TY_BOOL : TY_INVALID;
        default:     return TY_INVALID;
    }
}

static TypeTag infer_binary(Node *n, int *errors) {
    TypeTag L = infer(n->u.as_binary.left,  errors);
    TypeTag R = infer(n->u.as_binary.right, errors);
    switch (n->u.as_binary.op) {
        case BIN_ADD: case BIN_SUB: case BIN_MUL:
            return (is_numeric(L) && is_numeric(R)) ? ((L==TY_FLOAT||R==TY_FLOAT)?TY_FLOAT:TY_INT) : TY_INVALID;
        case BIN_DIV:
            return (is_numeric(L) && is_numeric(R)) ? TY_FLOAT : TY_INVALID;
        case BIN_LT: case BIN_LE: case BIN_GT: case BIN_GE:
            return (is_numeric(L) && is_numeric(R)) ? TY_BOOL : TY_INVALID;
        case BIN_EQ: case BIN_NEQ:
            if ((is_numeric(L)&&is_numeric(R)) || (L==TY_BOOL&&R==TY_BOOL) || (L==TY_STRING&&R==TY_STRING))
                return TY_BOOL;
            return TY_INVALID;
        case BIN_AND: case BIN_OR:
            return (L==TY_BOOL && R==TY_BOOL) ? TY_BOOL : TY_INVALID;
    }
    return TY_INVALID;
}

static TypeTag infer(Node *n, int *errors) {
    if (!n) return TY_INVALID;
    switch (n->kind) {
        case ND_INT:    return TY_INT;
        case ND_FLOAT:  return TY_FLOAT;
        case ND_BOOL:   return TY_BOOL;
        case ND_STRING: return TY_STRING;
        case ND_IDENT: {
            TypeTag t;
            if (!lookup_type(n->u.as_ident.name, &t)) {
                fprintf(stderr, "Erro semântico: identificador '%s' não declarado\n", n->u.as_ident.name);
                (*errors)++;
                return TY_INVALID;
            }
            return t;
        }
        case ND_UNARY:  return infer_unary(n, errors);
        case ND_BINARY: return infer_binary(n, errors);
        case ND_ASSIGN: {
            check_assign(n, errors);
            return infer(n->u.as_assign.value, errors);
        }

        case ND_EXPR:
            return infer(n->u.as_expr.expr, errors);

        case ND_CALL: {
        const FunSig *fs = find_fun(n->u.as_call.name);
        if (!fs) {
            fprintf(stderr, "Erro semântico: função '%s' não declarada\n", n->u.as_call.name);
            (*errors)++;
            for (size_t i=0;i<n->u.as_call.arg_count;i++) (void)infer(n->u.as_call.args[i], errors);
            return TY_INVALID;
        }
        if (fs->param_count != n->u.as_call.arg_count) {
            fprintf(stderr, "Erro semântico: chamada a '%s' com aridade incorreta (esperado %zu, obtido %zu)\n",
                    n->u.as_call.name, fs->param_count, n->u.as_call.arg_count);
            (*errors)++;
        }
        size_t m = (fs->param_count < n->u.as_call.arg_count) ? fs->param_count : n->u.as_call.arg_count;
        for (size_t i=0;i<m;i++) {
            TypeTag got = infer(n->u.as_call.args[i], errors);
            TypeTag want = fs->params[i];
            if (!(got==want || (is_numeric(got)&&is_numeric(want)))) {
                fprintf(stderr, "Erro semântico: argumento %zu de '%s' incompatível (esperado %d, obtido %d)\n",
                        i+1, n->u.as_call.name, (int)want, (int)got);
                (*errors)++;
            }
        }
        return fs->ret;
    }

        default:
            return TY_INVALID;
    }
}

/* =========================================================
 * Visitantes de statements (check_*)
 * ========================================================= */
static void check_block(Node *blk, int *errors) {
    push_scope_new();
    for (size_t i = 0; i < blk->u.as_block.count; i++)
        check_stmt(blk->u.as_block.stmts[i], errors);
    pop_scope();
}

static void check_decl(Node *decl, int *errors) {
    const char *name  = decl->u.as_decl.name;
    TypeTag     tdecl = decl->u.as_decl.type;

    Node *init = decl->u.as_decl.init;
    if (init) {
        TypeTag tinf = infer(init, errors);
        if (!(tinf == tdecl || (is_numeric(tinf) && is_numeric(tdecl)))) {
            fprintf(stderr, "Erro semântico: tipo incompatível na inicialização de '%s'\n", name);
            (*errors)++;
        }
    }

    if (exists_local(name)) {
        fprintf(stderr, "Erro semântico: variável '%s' já declarada neste escopo\n", name);
        (*errors)++;
        return;
    }

    (void)st_insert(g_scope->tab, name, tdecl, NULL);
}

static void check_assign(Node *as, int *errors) {
    const char *name = as->u.as_assign.name;
    Node *rhs = as->u.as_assign.value;

    TypeTag lhs_t;
    if (!lookup_type(name, &lhs_t)) {
        fprintf(stderr, "Erro semântico: variável '%s' não declarada\n", name);
        (*errors)++;
        return;
    }
    TypeTag rt = infer(rhs, errors);
    if (!(rt==lhs_t || (is_numeric(rt)&&is_numeric(lhs_t)))) {
        fprintf(stderr, "Erro semântico: tipos incompatíveis na atribuição a '%s'\n", name);
        (*errors)++;
    }
}

static void check_if(Node *ifn, int *errors) {
    if (!ifn->u.as_if.cond || infer(ifn->u.as_if.cond, errors) != TY_BOOL) {
        fprintf(stderr, "Erro semântico: condição do if deve ser bool\n");
        (*errors)++;
    }
    if (ifn->u.as_if.then_branch) check_stmt(ifn->u.as_if.then_branch, errors);
    if (ifn->u.as_if.else_branch) check_stmt(ifn->u.as_if.else_branch, errors);
}

static void check_while(Node *wn, int *errors) {
    if (!wn->u.as_while.cond || infer(wn->u.as_while.cond, errors) != TY_BOOL) {
        fprintf(stderr, "Erro semântico: condição do while deve ser bool\n");
        (*errors)++;
    }
    if (wn->u.as_while.body) check_stmt(wn->u.as_while.body, errors);
}

static void check_for(Node *fn, int *errors) {
    if (fn->u.as_for.init)  check_stmt(fn->u.as_for.init, errors);
    if (!fn->u.as_for.cond || infer(fn->u.as_for.cond, errors) != TY_BOOL) {
        fprintf(stderr, "Erro semântico: condição do for deve ser bool\n");
        (*errors)++;
    }
    if (fn->u.as_for.step)  check_stmt(fn->u.as_for.step, errors);
    if (fn->u.as_for.body)  check_stmt(fn->u.as_for.body, errors);
}

static void check_return(Node *rn, int *errors) {
    TypeTag want = current_fun_ret();
    if (!rn->u.as_return.expr) {
        if (want != TY_VOID) {
            fprintf(stderr, "Erro semântico: retorno vazio em função não-void\n");
            (*errors)++;
        }
        return;
    }
    if (want == TY_VOID) {
        fprintf(stderr, "Erro semântico: return com valor em função void\n");
        (*errors)++;
        return;
    }
    TypeTag got = infer(rn->u.as_return.expr, errors);
    if (!(got==want || (is_numeric(got)&&is_numeric(want)))) {
        fprintf(stderr, "Erro semântico: tipo de retorno incompatível\n");
        (*errors)++;
    }
}

static void check_function(Node *fn, int *errors) {
    register_fun(fn->u.as_function.name,
                 fn->u.as_function.ret_type,
                 fn->u.as_function.params,
                 fn->u.as_function.param_count);

    push_fun_ret(fn->u.as_function.ret_type);
    push_scope_new();

    for (size_t i = 0; i < fn->u.as_function.param_count; i++) {
        Node *pd = fn->u.as_function.params[i];
        st_insert(g_scope->tab, pd->u.as_decl.name, pd->u.as_decl.type, NULL);
    }

    check_stmt(fn->u.as_function.body, errors);

    pop_scope();
    pop_fun_ret();
}

static void check_stmt(Node *n, int *errors) {
    if (!n) return;
    switch (n->kind) {
        case ND_BLOCK:    check_block(n, errors); break;
        case ND_DECL:     check_decl(n, errors); break;
        case ND_ASSIGN:   check_assign(n, errors); break;
        case ND_IF:       check_if(n, errors); break;
        case ND_WHILE:    check_while(n, errors); break;
        case ND_FOR:      check_for(n, errors); break;
        case ND_RETURN:   check_return(n, errors); break;
        case ND_EXPR:     (void)infer(n->u.as_expr.expr, errors); break;
        case ND_FUNCTION: check_function(n, errors); break;
        default: /* ignorar outros nós por enquanto */ break;
    }
}

/* =========================================================
 * API pública
 * ========================================================= */
int check_semantics(Node *ast_root, SymbolTable *table) {
    int errors = 0;
    if (!ast_root) return 0;

    push_scope_with(table);
    check_stmt(ast_root, &errors);
    pop_scope();

    return errors;
}

int semantics_ok(Node *root, SymbolTable *table) {
    return check_semantics(root, table) == 0 ? 1 : 0;
}
