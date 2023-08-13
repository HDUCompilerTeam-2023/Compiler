#ifndef __IR_GEN_BASIC_BLOCK__
#define __IR_GEN_BASIC_BLOCK__
#include <ir/basic_block.h>
p_ir_basic_block ir_basic_block_gen();
p_ir_basic_block ir_basic_block_add_prev_target(p_ir_basic_block_branch_target p_target, p_ir_basic_block p_des);
void ir_basic_block_branch_del_prev_target(p_ir_basic_block_branch_target p_target);

void ir_basic_block_insert_prev(p_ir_basic_block p_prev, p_ir_basic_block p_next);
void ir_basic_block_insert_next(p_ir_basic_block p_next, p_ir_basic_block p_prev);

p_ir_basic_block ir_basic_block_addinstr_tail(p_ir_basic_block p_basic_block, p_ir_instr p_instr);
p_ir_basic_block ir_basic_block_addinstr_head(p_ir_basic_block p_basic_block, p_ir_instr p_instr);
void ir_basic_block_add_instr_list(p_ir_basic_block p_des_block, p_ir_basic_block p_src_block);

void ir_basic_block_set_target1(p_ir_basic_block p_basic_block, p_ir_basic_block_branch_target p_target);
void ir_basic_block_set_target2(p_ir_basic_block p_basic_block, p_ir_basic_block_branch_target p_target);
void ir_basic_block_set_br(p_ir_basic_block p_bb, p_ir_basic_block p_next);
void ir_basic_block_set_cond(p_ir_basic_block p_bb, p_ir_operand p_exp, p_ir_basic_block p_true, p_ir_basic_block p_false);
void ir_basic_block_set_ret(p_ir_basic_block p_bb, p_ir_operand p_exp);
void ir_basic_block_set_branch(p_ir_basic_block p_basic_block, p_ir_basic_block_branch p_branch);
void ir_basic_block_set_cond_exp(p_ir_basic_block p_basic_block, p_ir_operand p_exp);

void ir_basic_block_branch_target_clear_param(p_ir_basic_block_branch_target p_target);
void ir_basic_block_branch_target_clear_varray_param(p_ir_basic_block_branch_target p_target);
p_ir_basic_block_branch_target ir_basic_block_branch_target_gen(p_ir_basic_block p_block);
void ir_basic_block_branch_target_add_param(p_ir_basic_block_branch_target p_branch_target, p_ir_operand p_operand);
void ir_basic_block_branch_target_del_param(p_ir_basic_block_branch_target p_branch_target, p_ir_bb_param p_param);
p_ir_varray_bb_param ir_basic_block_branch_target_add_varray_param(p_ir_basic_block_branch_target p_branch_target, p_ir_varray_bb_phi p_phi, p_ir_varray_use p_use);
void ir_basic_block_branch_target_del_varray_param(p_ir_basic_block_branch_target p_branch_target, p_ir_varray_bb_param p_varray_param);

void ir_basic_block_add_dom_son(p_ir_basic_block p_basic_block, p_ir_basic_block p_son);
void ir_basic_block_add_phi(p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
void ir_basic_block_del_phi(p_ir_basic_block p_basic_block, p_ir_bb_phi p_bb_phi);
void ir_basic_block_clear_phi(p_ir_basic_block p_basic_block);
p_ir_varray_bb_phi ir_basic_block_add_varray_phi(p_ir_basic_block p_basic_block, p_ir_varray p_varray);
void ir_basic_block_del_varray_phi(p_ir_basic_block p_basic_block, p_ir_varray_bb_phi p_varray_bb_phi);
void ir_basic_block_clear_varray_phi(p_ir_basic_block p_basic_block);

void ir_basic_block_drop(p_ir_basic_block p_basic_block);
void ir_basic_block_branch_target_drop(p_ir_basic_block p_source_block, p_ir_basic_block_branch_target p_branch_target);

p_ir_basic_block_list ir_basic_block_list_init();
void ir_basic_block_list_clear(p_ir_basic_block_list p_block_list);
void ir_basic_block_list_add(p_ir_basic_block_list p_list, p_ir_basic_block p_basic_block);
void ir_basic_block_list_drop(p_ir_basic_block_list p_basic_block_list);
p_ir_basic_block_list_node ir_basic_block_list_node_gen(p_ir_basic_block p_basic_block);
void ir_basic_block_list_node_set_basic_block(p_ir_basic_block_list_node p_basic_block_list_node, p_ir_basic_block p_basic_block);
void ir_basic_block_list_node_drop(p_ir_basic_block_list_node p_basic_block_list_node);
void copy_basic_block_list(p_ir_basic_block_list p_des, p_ir_basic_block_list p_src);
bool if_in_basic_block_list(p_ir_basic_block_list p_list, p_ir_basic_block p_block);
void ir_basic_block_list_del(p_ir_basic_block_list p_list, p_ir_basic_block p_block);

void ir_branch_target_node_drop(p_ir_branch_target_node p_del);
#endif
