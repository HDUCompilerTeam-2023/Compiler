#ifndef __HIR2MIR_STMT__
#define __HIR2MIR_STMT__

#include <hir2mir/info_gen.h>
void hir2mir_stmt_gen(p_hir2mir_info p_info, p_mir_basic_block while_start, p_mir_basic_block while_end_next, p_hir_stmt p_stmt);

void hir2mir_stmt_return_gen(p_hir2mir_info p_info, p_hir_exp p_exp);
void hir2mir_stmt_exp_gen(p_hir2mir_info p_info, p_hir_exp p_exp);
void hir2mir_stmt_break_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_end_next);
void hir2mir_stmt_continue_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_start);
void hir2mir_stmt_if_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_start, p_mir_basic_block p_while_end_next, p_hir_exp p_exp, p_hir_stmt p_stmt_1);
void hir2mir_stmt_if_else_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_start, p_mir_basic_block p_while_end_next, p_hir_exp p_exp, p_hir_stmt p_stmt_1, p_hir_stmt p_stmt_2);
void hir2mir_stmt_while_gen(p_hir2mir_info p_info, p_hir_exp p_exp, p_hir_stmt p_stmt_1);
void hir2mir_stmt_assign_gen(p_hir2mir_info p_info, p_hir_exp p_lval, p_hir_exp p_rval);
void hir2mir_stmt_block_gen(p_hir2mir_info p_info,
    p_mir_basic_block while_start, p_mir_basic_block while_end_next, p_hir_block p_block);
#endif