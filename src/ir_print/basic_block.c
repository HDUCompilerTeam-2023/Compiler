#include "ir_gen/basic_block.h" // 头文件包含还需考虑
#include <ir/basic_block.h>
#include <ir/bb_param.h>
#include <ir/instr.h>
#include <ir/operand.h>
#include <ir_print.h>

#include <stdio.h>

static inline void ir_basic_block_branch_print(p_ir_basic_block_branch p_branch) {
    printf("    ");
    switch (p_branch->kind) {
    case ir_abort_branch:
        printf("abort");
        break;
    case ir_br_branch:
        printf("br ");
        ir_basic_block_branch_target_print(p_branch->p_target_1);
        assert(!p_branch->p_exp);
        assert(!p_branch->p_target_2);
        break;
    case ir_cond_branch:
        printf("br ");
        ir_operand_print(p_branch->p_exp);
        printf(", ");
        ir_basic_block_branch_target_print(p_branch->p_target_1);
        printf(", ");
        ir_basic_block_branch_target_print(p_branch->p_target_2);
        break;
    case ir_ret_branch:
        printf("ret");
        if (p_branch->p_exp) {
            printf(" ");
            ir_operand_print(p_branch->p_exp);
        }
        assert(!p_branch->p_target_1);
        assert(!p_branch->p_target_2);
        break;
    }
    printf("\n");
}

void ir_basic_block_print(p_ir_basic_block p_basic_block) {
    assert(p_basic_block);

    printf("b%ld", p_basic_block->block_id);
    if (!list_head_alone(&p_basic_block->basic_block_phis->bb_phi))
        ir_bb_phi_list_print(p_basic_block->basic_block_phis);
    printf(":");

    if (!list_head_alone(&p_basic_block->prev_basic_block_list)) {
        printf("                        ; preds = ");
        p_list_head p_node;
        list_for_each(p_node, &p_basic_block->prev_basic_block_list) {
            size_t id = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block->block_id;
            printf("b%ld", id);
            if (p_node->p_next != &p_basic_block->prev_basic_block_list)
                printf(", ");
        }
    }
    printf("\n");

    p_list_head p_node;
    p_ir_instr p_instr = NULL;
    list_for_each(p_node, &p_basic_block->instr_list) {
        p_instr = list_entry(p_node, ir_instr, node);
        ir_instr_print(p_instr);
    }
    ir_basic_block_branch_print(p_basic_block->p_branch);
}

void ir_basic_block_branch_target_print(p_ir_basic_block_branch_target p_branch_target) {
    printf("b%ld", p_branch_target->p_block->block_id);
    if (!list_head_alone(&p_branch_target->p_block_param->bb_param))
        ir_bb_param_list_print(p_branch_target->p_block_param);
}

void ir_basic_block_dom_info_print(p_ir_basic_block p_basic_block, size_t depth) {
    for (size_t i = 0; i < depth; i++)
        printf("-");
    printf("b%ld (dfn_id: %ld)\n", p_basic_block->block_id, p_basic_block->dfn_id);
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->dom_son_list) {
        p_ir_basic_block p_son = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        ir_basic_block_dom_info_print(p_son, depth + 4);
    }
}
