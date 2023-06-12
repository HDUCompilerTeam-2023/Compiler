#ifndef __OPTIMIZER__CONVERT_SSA__
#define __OPTIMIZER__CONVERT_SSA__

#include <ir_gen.h>
#include <program/use.h>
typedef struct convert_ssa convert_ssa, *p_convert_ssa;
typedef struct operand_stack_node operand_stack_node, *p_operand_stack_node;
typedef struct ssa_var_info ssa_var_info, *p_ssa_var_info;
typedef struct ssa_var_list_info ssa_var_list_info, *p_ssa_var_list_info;
typedef struct sym_stack_node sym_stack_node, *p_sym_stack_node;

struct convert_ssa {
    p_ir_basic_block p_basic_block;
    p_bitmap dom_frontier; // 支配边界

    p_bitmap p_phi_var; // phi 函数变量集合
    p_bitmap p_def_var; // 定值集合

    bool if_in; // 是否在工作表中
};

struct ssa_var_info {
    p_symbol_var p_vmem;
    p_ir_vreg p_current_vreg;
    list_head sym_stack; // 保存进入基本块时的信息
};

struct sym_stack_node {
    list_head node;
    p_ir_vreg p_vreg;
};

struct ssa_var_list_info {
    p_ssa_var_info p_base;
    size_t vmem_num;
    p_symbol_func p_func;
};

void convert_ssa_gen(convert_ssa *dfs_seq, size_t block_num, size_t var_num, p_ir_basic_block p_basic_block, size_t current_num);
size_t convert_ssa_init_dfs_sequence(convert_ssa *dfs_seq, size_t var_num, size_t block_num, p_ir_basic_block p_entry, size_t current_num);
p_ssa_var_list_info convert_ssa_init_var_list(p_symbol_func p_func);

void convert_ssa_compute_dom_frontier(convert_ssa *dfs_seq, size_t block_num);

void convert_ssa_insert_phi(p_convert_ssa dfs_seq, size_t block_num, p_ssa_var_list_info p_var_list);

void convert_ssa_rename_var(p_ssa_var_list_info p_var_list, p_convert_ssa dfs_seq, p_ir_basic_block p_entry);

void convert_ssa_func(p_symbol_func p_func);
void convert_ssa_program(p_program p_program);

void convert_ssa_dfs_seq_drop(convert_ssa *dfs_seq, size_t block_num);
void ssa_var_list_info_drop(p_ssa_var_list_info p_info);

#endif
