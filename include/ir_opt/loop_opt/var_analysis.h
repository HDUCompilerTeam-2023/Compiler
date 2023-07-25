#ifndef __IR_OPT_LOOP_OPT_VAR_ANALYSIS__
#define __IR_OPT_LOOP_OPT_VAR_ANALYSIS__

#include <program/def.h>
#include <program/use.h>

#include <ir_manager/buildnestree.h>
#include <symbol/func.h>

void program_var_analysis(p_program p_program);
void loop_var_analysis(p_nestedtree_node root);
void invariant_analysis(p_nestedtree_node root);
void step_var_analysis(p_nestedtree_node root);
void basic_var_analysis(p_nestedtree_node root);

bool check_operand(p_ir_operand p_operand);
#endif