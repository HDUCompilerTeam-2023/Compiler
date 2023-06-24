#ifndef __IR_BASIC_BLOCK__
#define __IR_BASIC_BLOCK__

#include <ir.h>

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
};

struct ir_basic_block {
    size_t block_id;
    list_head instr_list;
    p_ir_basic_block_branch p_branch;

    list_head prev_basic_block_list;
    list_head node;

    // SSA 相关
    p_ir_bb_phi_list basic_block_phis; // 基本块参数列表
    p_ir_basic_block p_dom_parent; // 支配树上的父亲
    list_head dom_son_list; // 支配树上的儿子

    size_t dfn_id; // 深度优先序
    bool if_visited;
};

struct ir_basic_block_list_node {
    p_ir_basic_block p_basic_block;
    list_head node;
};

struct ir_basic_block_branch_target {
    p_ir_basic_block p_block;
    p_ir_bb_param_list p_block_param;
};
#endif