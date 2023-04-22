#ifndef __HIR_GEN_STMT__
#define __HIR_GEN_STMT__

#include <hir/stmt.h>

p_hir_stmt hir_stmt_return_gen(p_hir_exp p_exp);
p_hir_stmt hir_stmt_exp_gen(p_hir_exp p_exp);
p_hir_stmt hir_stmt_break_gen(void);
p_hir_stmt hir_stmt_continue_gen(void);
p_hir_stmt hir_stmt_if_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1);
p_hir_stmt hir_stmt_if_else_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1, p_hir_stmt p_stmt_2);
p_hir_stmt hir_stmt_while_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1);
p_hir_stmt hir_stmt_do_while_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1);
p_hir_stmt hir_stmt_block_gen(p_hir_block p_block);
void hir_stmt_drop(p_hir_stmt p_stmt);

#endif