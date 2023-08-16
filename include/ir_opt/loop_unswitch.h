#ifndef __IR_OPT_LOOP_UNSWITCH__
#define __IR_OPT_LOOP_UNSWITCH__

#include <ir_manager/buildnestree.h>
#include <ir_opt/code_copy.h>

void ir_opt_loop_unswitch(p_program p_program);
bool loop_unswitch_try(p_nestedtree_node root);
void loop_unswitch(p_nestedtree_node root, p_ir_basic_block p_switch_block);

#endif