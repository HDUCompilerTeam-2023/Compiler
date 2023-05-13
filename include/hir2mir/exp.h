#ifndef __HIR2MIR_INSTR__
#define __HIR2MIR_INSTR__

#include <hir2mir/info_gen.h>

p_mir_operand hir2mir_exp_gen(p_hir2mir_info p_info, p_hir_exp p_exp);
p_mir_operand hir2mir_exp_cond_gen(p_hir2mir_info p_info, p_mir_basic_block p_true_block, p_mir_basic_block p_false_block, p_hir_exp p_exp);

#endif