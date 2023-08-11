#ifndef __IR_OPT_SCEV__
#define __IR_OPT_SCEV__

#include <ir.h>
#include <ir_manager/buildnestree.h>
#include <program/def.h>

typedef struct sum_instr_node sum_instr_node, *p_sum_instr_node;
typedef struct sum_info sum_info, *p_sum_info;

struct sum_instr_node {
    p_ir_instr p_instr;
    list_head node;
};

struct sum_info {
    p_ir_vreg count;
    p_ir_vreg mul_count;
    p_ir_vreg add_const;
    p_ir_vreg add_var;
    list_head instr_list;
};

bool check_operand(p_ir_operand p_operand);
bool cmp_vreg(p_ir_operand p_operand1, p_ir_vreg p_vreg);
void dfs_loop_block(HashTable *hash, p_ir_basic_block p_basic_block);

void instr_insert(ir_binary_op op, p_ir_operand p1, p_ir_operand p2, p_ir_vreg p_des, p_ir_basic_block p_block);
void scev_add_instr(p_ir_vreg p_vreg, p_sum_info p_info, p_ir_basic_block p_block, bool is_sub);
void reduction_var(p_nestedtree_node root, p_ir_basic_block p_basic_block, p_ir_basic_block_branch_target p_target, p_list_head target_list);

void program_var_analysis(p_program p_program, bool if_opt);
void loop_var_analysis(p_nestedtree_node root, bool if_opt);
void invariant_analysis(p_nestedtree_node root);
void basic_var_analysis(p_nestedtree_node root);
void induction_var_analysis(p_nestedtree_node root);
void loop_info_analysis(p_nestedtree_node root);
void accumulation_analysis(p_nestedtree_node root);
void var_strength_reduction(p_nestedtree_node root);

void secv_drop(p_nestedtree_node root);
#endif