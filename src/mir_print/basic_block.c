
#include "mir_gen/basic_block.h" // 头文件包含还需考虑
#include <mir/basic_block.h>
#include <mir/bb_param.h>
#include <mir/instr.h>
#include <mir/operand.h>
#include <mir_print.h>

#include <stdio.h>
void mir_basic_block_print(p_mir_basic_block p_basic_block) {
    assert(p_basic_block);

    printf("b%ld", p_basic_block->block_id);
    if (!list_head_alone(&p_basic_block->basic_block_phis->bb_phi))
        mir_bb_phi_list_print(p_basic_block->basic_block_phis);
    printf(":");

    if (!list_head_alone(&p_basic_block->prev_basic_block_list)) {
        printf("                        ; preds = ");
        p_list_head p_node;
        list_for_each(p_node, &p_basic_block->prev_basic_block_list) {
            size_t id = list_entry(p_node, mir_basic_block_list_node, node)->p_basic_block->block_id;
            printf("b%ld", id);
            if (p_node->p_next != &p_basic_block->prev_basic_block_list)
                printf(", ");
        }
    }
    printf("\n");

    p_list_head p_node;
    p_mir_instr p_instr = NULL;
    list_for_each(p_node, &p_basic_block->instr_list) {
        p_instr = list_entry(p_node, mir_instr, node);
        mir_instr_print(p_instr);
    }
}

void mir_basic_block_call_print(p_mir_basic_block_call p_block_call) {
    printf("b%ld", p_block_call->p_block->block_id);
    if (!list_head_alone(&p_block_call->p_block_param->bb_param))
        mir_bb_param_list_print(p_block_call->p_block_param);
}

void mir_basic_block_dom_info_print(p_mir_basic_block p_basic_block, size_t depth) {
    for (size_t i = 0; i < depth; i++)
        printf("-");
    printf("b%ld (dfn_id: %ld)\n", p_basic_block->block_id, p_basic_block->dfn_id);
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->dom_son_list) {
        p_mir_basic_block p_son = list_entry(p_node, mir_basic_block_list_node, node)->p_basic_block;
        mir_basic_block_dom_info_print(p_son, depth + 4);
    }
}
