#ifndef __IR_GEN_BASIC_BLOCK__
#define __IR_GEN_BASIC_BLOCK__
#include <ir/basic_block.h>
p_ir_basic_block ir_basic_block_gen();
p_ir_basic_block ir_basic_block_add_prev(p_ir_basic_block p_prev, p_ir_basic_block p_next);

p_ir_basic_block ir_basic_block_addinstr(p_ir_basic_block p_basic_block, p_ir_instr p_instr);

void ir_basic_block_set_br(p_ir_basic_block p_bb, p_ir_basic_block p_next);
void ir_basic_block_set_cond(p_ir_basic_block p_bb, p_ir_operand p_exp, p_ir_basic_block p_true, p_ir_basic_block p_false);
void ir_basic_block_set_ret(p_ir_basic_block p_bb, p_ir_operand p_exp);

p_ir_basic_block_branch_target ir_basic_block_branch_target_gen(p_ir_basic_block p_block);
void ir_basic_block_branch_target_add_param(p_ir_basic_block_branch_target p_branch_target, p_ir_operand p_operand);

void ir_basic_block_add_dom_son(p_ir_basic_block p_basic_block, p_ir_basic_block p_son);
void ir_basic_block_add_param(p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);

void ir_basic_block_drop(p_ir_basic_block p_basic_block);
void ir_basic_block_branch_target_drop(p_ir_basic_block_branch_target p_branch_target);
#endif
