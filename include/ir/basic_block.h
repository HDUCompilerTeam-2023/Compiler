#ifndef __IR_BASIC_BLOCK__
#define __IR_BASIC_BLOCK__

#include <ir.h>
#include <ir_manager/buildnestree.h>

enum ir_basic_block_branch_kind {
    ir_abort_branch,
    ir_ret_branch,
    ir_br_branch,
    ir_cond_branch,
};

struct ir_basic_block_branch {
    ir_basic_block_branch_kind kind;
    p_ir_basic_block_branch_target p_target_1, p_target_2;
    p_ir_operand p_exp;
    p_ir_vreg_list p_live_in;
};

struct ir_basic_block {
    size_t block_id;
    list_head instr_list;
    size_t instr_num;
    p_ir_basic_block_branch p_branch;

    list_head prev_branch_target_list;
    size_t prev_num;
    list_head node;

    list_head seqs_node;
    // SSA 相关
    list_head basic_block_phis; // 基本块参数列表
    list_head varray_basic_block_phis;
    p_ir_basic_block p_dom_parent; // 支配树上的父亲
    p_ir_basic_block_list dom_son_list; // 支配树上的儿子
    size_t dom_depth; // 支配树深度

    p_nestedtree_node p_nestree_node;
    list_head loop_node_list;
    list_head target1_scc_list, target2_scc_list;
    struct RedBlackTree *loop_check;
    bool is_loop_head;
    bool is_loop_back;
    bool is_loop_exit;
    size_t nestred_depth;

    p_ir_vreg_list p_live_in;
    p_ir_vreg_list p_live_out;

    p_symbol_func p_func;
    bool if_visited;

    void *p_info;
    size_t max_reg_pres_r;
    size_t max_reg_pres_s;
};
struct ir_branch_target_node {
    p_ir_basic_block_branch_target p_target;
    list_head node;
};
struct ir_basic_block_list {
    list_head block_list;
};
struct ir_basic_block_list_node {
    p_ir_basic_block p_basic_block;
    list_head node;
};

struct ir_basic_block_branch_target {
    p_ir_basic_block p_source_block;
    p_ir_basic_block p_block;
    list_head block_param;
    list_head varray_bb_param;
};
#endif
