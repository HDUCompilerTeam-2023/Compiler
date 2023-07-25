#ifndef __IR_OPT_SCEV__
#define __IR_OPT_SCEV__

#include <ir.h>
#include <ir_manager/buildnestree.h>
#include <program/def.h>

bool check_operand(p_ir_operand p_operand);

void program_var_analysis(p_program p_program);
void loop_var_analysis(p_nestedtree_node root);
void invariant_analysis(p_nestedtree_node root);
void basic_var_analysis(p_nestedtree_node root);
#endif