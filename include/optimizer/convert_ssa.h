#ifndef __OPTIMIZER__CONVERT_SSA__
#define __OPTIMIZER__CONVERT_SSA__

#include <mir_gen.h>

struct convert_ssa{
    p_mir_basic_block p_basic_block; 
    p_bitmap dom_frontier; // 支配边界

    p_bitmap p_phi_var; // phi 函数变量集合
    p_bitmap p_def_var; // 定值集合

    bool if_in; // 是否在工作表中
};


struct operand_stack_node{
    list_head node;
    p_mir_operand p_operand;
};

struct ssa_var_info{
    p_mir_operand p_operand;
    list_head stack;
    size_t count;
};
typedef struct convert_ssa convert_ssa, *p_convert_ssa;
typedef struct operand_stack_node operand_stack_node, *p_operand_stack_node;
typedef struct ssa_var_info ssa_var_info, *p_ssa_var_info;

void convert_ssa_gen(convert_ssa *dfs_seq, size_t block_num, size_t var_num, p_mir_basic_block p_basic_block, size_t current_num);
size_t convert_ssa_init_dfs_sequence(convert_ssa *dfs_seq, size_t var_num, size_t block_num, p_mir_basic_block p_entry, size_t current_num);
void convert_ssa_init_var_list(p_ssa_var_info p_var_list, size_t var_num, p_mir_func p_func, p_mir_temp_sym p_ret);

void convert_ssa_compute_dom_frontier(convert_ssa *dfs_seq, size_t block_num);

void convert_ssa_insert_phi(p_convert_ssa dfs_seq, size_t block_num, p_mir_temp_sym p_ret, size_t var_num);
void convert_ssa_rewrite_phi(p_convert_ssa dfs_seq, size_t block_num, p_ssa_var_info p_var_list, size_t var_num);

void convert_ssa_func(p_mir_func p_func);
void convert_ssa_program(p_mir_program p_program);

void convert_ssa_dfs_seq_drop(convert_ssa *dfs_seq, size_t block_num);
void ssa_var_info_drop(p_ssa_var_info p_info, size_t var_num);

#endif