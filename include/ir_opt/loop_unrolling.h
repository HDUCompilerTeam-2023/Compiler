#ifndef __IR_OPT_LOOP_UNROLLING__
#define __IR_OPT_LOOP_UNROLLING__

#include <ir_manager/buildnestree.h>
#include <ir_opt/code_copy.h>

void loop_block_vreg_copy(p_ir_basic_block p_block, p_copy_map p_map);

void loop_unrolling(p_nestedtree_node p_root, int k, bool is_full);
void prev_loop_add(p_nestedtree_node root, int k);

void ir_opt_loop_full_unrolling(p_program p_program);
void loop_full_unrolling(p_nestedtree_node root);

void ir_opt_loop_unrolling(p_program p_program, int unrolling_time);
void heap_loop_add(p_nestedtree_node root);
#endif