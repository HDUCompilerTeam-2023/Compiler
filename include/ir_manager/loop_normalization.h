#ifndef __IR_MANAGER_LOOP_NORMALIZATION__
#define __IR_MANAGER_LOOP_NORMALIZATION__

#include <ir/basic_block.h>

#include <ir_manager/buildnestree.h>

void program_loop_normalization(p_program p_program);
void loop_normalization(p_nestedtree_node root);

p_ir_basic_block ir_basic_block_target_split(p_list_head p_list, p_ir_basic_block p_block, bool is_prev);

void loop_lcssa(p_nestedtree_node root);
p_ir_basic_block loop_pre_block_add(p_nestedtree_node root);
void loop_exit_block_add(p_nestedtree_node root);
p_ir_basic_block loop_latch_block_add(p_nestedtree_node root);
#endif