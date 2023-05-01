#ifndef __OPTIMIZER__CONVERT_SSA__
#define __OPTIMIZER__CONVERT_SSA__

#include <mir_gen.h>
typedef struct convert_ssa convert_ssa, *p_convert_ssa;
typedef struct operand_stack_node operand_stack_node, *p_operand_stack_node;
typedef struct ssa_var_info ssa_var_info, *p_ssa_var_info;
typedef struct ssa_var_list_info ssa_var_list_info, *p_ssa_var_list_info;

struct convert_ssa{
    p_mir_basic_block p_basic_block; 
    p_bitmap dom_frontier; // 支配边界

    p_bitmap p_phi_var; // phi 函数变量集合
    p_bitmap p_def_var; // 定值集合

    bool if_in; // 是否在工作表中
};


struct ssa_var_info{
    p_mir_operand p_operand;
    size_t current_count;
    size_t count;
};

struct ssa_var_list_info{
    p_ssa_var_info p_base;
    size_t global_num;
    size_t local_num;
    size_t temp_num;
    p_mir_temp_sym p_ret_sym;
};


void convert_ssa_gen(convert_ssa *dfs_seq, size_t block_num, size_t var_num, p_mir_basic_block p_basic_block, size_t current_num);
size_t convert_ssa_init_dfs_sequence(convert_ssa *dfs_seq, size_t var_num, size_t block_num, p_mir_basic_block p_entry, size_t current_num);
p_ssa_var_list_info convert_ssa_init_var_list(p_mir_func p_func, p_mir_program p_program);

void convert_ssa_compute_dom_frontier(convert_ssa *dfs_seq, size_t block_num);

void convert_ssa_insert_phi(p_convert_ssa dfs_seq, size_t block_num, p_ssa_var_list_info p_var_list);

void convert_ssa_rename_var(p_ssa_var_list_info p_var_list, p_convert_ssa dfs_seq, p_mir_basic_block p_entry);

void convert_ssa_func(p_mir_func p_func, p_mir_program p_program);
void convert_ssa_program(p_mir_program p_program);

void convert_ssa_dfs_seq_drop(convert_ssa *dfs_seq, size_t block_num);
void ssa_var_list_info_drop(p_ssa_var_list_info p_info);

#endif