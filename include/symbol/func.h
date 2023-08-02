#ifndef __SYMBOL_FUNC__
#define __SYMBOL_FUNC__

#include <ir.h>
#include <ir_manager/builddomtree.h>
#include <ir_manager/buildnestree.h>
#include <ir_manager/call_graph.h>
#include <ir_manager/side_effects.h>
#include <symbol.h>
struct symbol_func {
    // type info
    bool is_va;
    basic_type ret_type;

    char *name;
    uint64_t id;

    size_t var_cnt;
    list_head variable;

    size_t param_reg_cnt;
    list_head param_reg_list;

    size_t vreg_cnt;
    list_head vreg_list;

    size_t block_cnt;
    list_head block;
    p_ir_basic_block p_ret_block;
    p_ir_basic_block p_entry_block;

    list_head param;
    list_head call_param_vmem_list;
    size_t stack_size;
    size_t inner_stack_size;

    p_nestedtree_node p_nestedtree_root;

    p_call_graph_node p_call_graph_node;
    p_func_side_effects p_side_effects;

    size_t instr_num;

    size_t use_reg_num_r;
    size_t use_reg_num_s;
    size_t save_reg_r_num;
    size_t save_reg_s_num;
    list_head node;

    bool if_updated_graph;
};

#endif
