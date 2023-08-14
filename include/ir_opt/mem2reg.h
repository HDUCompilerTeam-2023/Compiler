#ifndef __OPTIMIZER__CONVERT_SSA__
#define __OPTIMIZER__CONVERT_SSA__

#include <ir_gen.h>
#include <program/use.h>
typedef struct convert_ssa convert_ssa, *p_convert_ssa;
typedef struct operand_stack_node operand_stack_node, *p_operand_stack_node;
typedef struct ssa_var_info ssa_var_info, *p_ssa_var_info;
typedef struct ssa_var_list_info ssa_var_list_info, *p_ssa_var_list_info;
typedef struct sym_stack_node sym_stack_node, *p_sym_stack_node;
typedef struct convert_ssa_list convert_ssa_list, *p_convert_ssa_list;

struct convert_ssa {
    p_bitmap dom_frontier; // 支配边界

    p_bitmap p_phi_var; // phi 函数变量集合
    p_bitmap p_def_var; // 定值集合

    p_convert_ssa p_prev;
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

struct convert_ssa_list {
    p_convert_ssa p_base;
    p_symbol_func p_func;
};

struct ssa_var_list_info {
    p_ssa_var_info p_base;
    size_t vmem_num;
    p_symbol_func p_func;
};

p_convert_ssa_list mem2reg_info_gen(p_symbol_func p_func);
p_ssa_var_list_info mem2reg_init_var_list(p_symbol_func p_func);

void mem2reg_compute_dom_frontier(p_convert_ssa_list p_convert_list);

void mem2reg_insert_phi(p_convert_ssa_list p_convert_list, p_ssa_var_list_info p_var_list);

void mem2reg_rename_var(p_ssa_var_list_info p_var_list, p_convert_ssa_list p_convert_list, p_ir_basic_block p_entry);

void mem2reg_func_pass(p_symbol_func p_func);
void mem2reg_program_pass(p_program p_program);

void convert_ssa_dfs_seq_drop(p_convert_ssa_list p_convert_list);
void ssa_var_list_info_drop(p_ssa_var_list_info p_info);

#endif
