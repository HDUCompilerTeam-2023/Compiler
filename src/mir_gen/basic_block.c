
#include <mir_gen.h>
#include <mir_gen/basic_block.h>

p_mir_basic_block mir_basic_block_gen() {
    p_mir_basic_block p_mir_block = malloc(sizeof(*p_mir_block));
    *p_mir_block = (mir_basic_block) {
        .instr_list = list_head_init(&p_mir_block->instr_list),
        .prev_basic_block_list = list_head_init(&p_mir_block->prev_basic_block_list),
        .block_id = 0,
        .node = list_head_init(&p_mir_block->node),
        .basic_block_parameters = list_head_init(&p_mir_block->basic_block_parameters),
        .dom_son_list = list_head_init(&p_mir_block->dom_son_list),
        .p_dom_parent = NULL,
        .if_visited = false,
    };
    return p_mir_block;
}
// 插入前驱节点列表
p_mir_basic_block mir_basic_block_add_prev(p_mir_basic_block p_prev, p_mir_basic_block p_next) {
    p_list_head p_node;
    list_for_each(p_node, &p_next->prev_basic_block_list) {
        if (list_entry(p_node, mir_basic_block, node) == p_prev) // 若已存在不插入
            return NULL;
    }
    p_mir_basic_block_list_node node = malloc(sizeof(*node));
    *node = (mir_basic_block_list_node) {
        .p_basic_block = p_prev,
        .node = list_head_init(&node->node),
    };
    list_add_prev(&node->node, &p_next->prev_basic_block_list);
    return p_next;
}

p_mir_basic_block mir_basic_block_addinstr(p_mir_basic_block p_basic_block, p_mir_instr p_instr) {
    list_add_prev(&p_instr->node, &p_basic_block->instr_list);
    return p_basic_block;
}

void mir_basic_block_add_dom_son(p_mir_basic_block p_basic_block, p_mir_basic_block p_son) {
    p_mir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (mir_basic_block_list_node) {
        .p_basic_block = p_son,
        .node = list_head_init(&p_new_node->node),
    };
    p_son->p_dom_parent = p_basic_block;
    list_add_prev(&p_new_node->node, &p_basic_block->dom_son_list);
}

void mir_basic_block_add_param(p_mir_basic_block p_basic_block, p_mir_operand p_operand) {
    p_mir_basic_block_params_node p_param = malloc(sizeof(*p_param));
    *p_param = (mir_basic_block_params_node) {
        .node = list_head_init(&p_param->node),
        .p_operand = p_operand,
    };
    list_add_prev(&p_param->node, &p_basic_block->basic_block_parameters);
}

void mir_basic_block_drop(p_mir_basic_block p_basic_block) {
    assert(p_basic_block);
    while (!list_head_alone(&p_basic_block->instr_list)) {
        p_mir_instr p_instr = list_entry(p_basic_block->instr_list.p_next, mir_instr, node);
        list_del(&p_instr->node);
        mir_instr_drop(p_instr);
    }
    while (!list_head_alone(&p_basic_block->prev_basic_block_list)) {
        p_mir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block->prev_basic_block_list.p_next, mir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        free(p_basic_block_list_node);
    }
    while (!list_head_alone(&p_basic_block->dom_son_list)) {
        p_mir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block->dom_son_list.p_next, mir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        free(p_basic_block_list_node);
    }
    while (!list_head_alone(&p_basic_block->basic_block_parameters)) {
        p_mir_basic_block_params_node p_param = list_entry(p_basic_block->basic_block_parameters.p_next, mir_basic_block_params_node, node);
        list_del(&p_param->node);
        mir_operand_drop(p_param->p_operand);
        free(p_param);
    }
    free(p_basic_block);
}
