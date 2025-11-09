#ifndef IR_BUILDER_H
#define IR_BUILDER_H

#include "ast_base.h"
#include "symbol_table.h"
#include "ir.h"

/** Reseta estado interno do builder (map de variáveis -> temporários).
 *  Chame isso sempre que começar a gerar IR de uma nova função.
 */
void irb_reset_state(void);

/** Gera IR para um statement */
void irb_emit_stmt(IrFunc *f, Node *stmt);

/** Gera IR para uma expressão e retorna o temporário tN com o resultado */
int  irb_emit_expr(IrFunc *f, Node *expr);

#endif
