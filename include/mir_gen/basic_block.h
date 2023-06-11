#ifndef __MIR_GEN_BASIC_BLOCK__
#define __MIR_GEN_BASIC_BLOCK__
#include <mir/basic_block.h>
p_mir_basic_block mir_basic_block_gen();
p_mir_basic_block mir_basic_block_add_prev(p_mir_basic_block p_prev, p_mir_basic_block p_next);

p_mir_basic_block mir_basic_block_addinstr(p_mir_basic_block p_basic_block, p_mir_instr p_instr);

void mir_basic_block_set_br(p_mir_basic_block p_bb, p_mir_basic_block p_next);
void mir_basic_block_set_cond(p_mir_basic_block p_bb, p_mir_operand p_exp, p_mir_basic_block p_true, p_mir_basic_block p_false);
void mir_basic_block_set_ret(p_mir_basic_block p_bb, p_mir_operand p_exp);

p_mir_basic_block_branch_target mir_basic_block_branch_target_gen(p_mir_basic_block p_block);
void mir_basic_block_branch_target_add_param(p_mir_basic_block_branch_target p_branch_target, p_mir_operand p_operand);

void mir_basic_block_add_dom_son(p_mir_basic_block p_basic_block, p_mir_basic_block p_son);
void mir_basic_block_add_param(p_mir_basic_block p_basic_block, p_mir_vreg p_vreg);

void mir_basic_block_drop(p_mir_basic_block p_basic_block);
void mir_basic_block_branch_target_drop(p_mir_basic_block_branch_target p_branch_target);
#endif
