#include <ir_gen.h>
#include <ir_gen/basic_block.h>

static inline p_ir_basic_block_branch ir_basic_block_branch_gen() {
    p_ir_basic_block_branch p_branch = malloc(sizeof(*p_branch));
    *p_branch = (ir_basic_block_branch) {
        .kind = ir_abort_branch,
        .p_exp = NULL,
        .p_target_1 = NULL,
        .p_target_2 = NULL,
    };
    return p_branch;
}
static inline void ir_basic_block_branch_drop(p_ir_basic_block_branch p_branch) {
    assert(p_branch);
    if (p_branch->kind == ir_ret_branch && p_branch->p_exp)
        ir_operand_drop(p_branch->p_exp);
    if (p_branch->kind == ir_br_branch)
        ir_basic_block_branch_target_drop(p_branch->p_target_1);
    if (p_branch->kind == ir_cond_branch) {
        ir_operand_drop(p_branch->p_exp);
        ir_basic_block_branch_target_drop(p_branch->p_target_1);
        ir_basic_block_branch_target_drop(p_branch->p_target_2);
    }
    free(p_branch);
}

p_ir_basic_block ir_basic_block_gen() {
    p_ir_basic_block p_ir_block = malloc(sizeof(*p_ir_block));
    *p_ir_block = (ir_basic_block) {
        .instr_list = list_head_init(&p_ir_block->instr_list),
        .prev_basic_block_list = list_head_init(&p_ir_block->prev_basic_block_list),
        .p_branch = ir_basic_block_branch_gen(),
        .block_id = 0,
        .node = list_head_init(&p_ir_block->node),
        .basic_block_phis = ir_bb_phi_list_init(),
        .dom_son_list = list_head_init(&p_ir_block->dom_son_list),
        .p_dom_parent = NULL,
        .if_visited = false,
    };
    return p_ir_block;
}
// 插入前驱节点列表
p_ir_basic_block ir_basic_block_add_prev(p_ir_basic_block p_prev, p_ir_basic_block p_next) {
    p_list_head p_node;
    list_for_each(p_node, &p_next->prev_basic_block_list) {
        if (list_entry(p_node, ir_basic_block, node) == p_prev) // 若已存在不插入
            return NULL;
    }
    p_ir_basic_block_list_node node = malloc(sizeof(*node));
    *node = (ir_basic_block_list_node) {
        .p_basic_block = p_prev,
        .node = list_head_init(&node->node),
    };
    list_add_prev(&node->node, &p_next->prev_basic_block_list);
    return p_next;
}

p_ir_basic_block ir_basic_block_addinstr(p_ir_basic_block p_basic_block, p_ir_instr p_instr) {
    list_add_prev(&p_instr->node, &p_basic_block->instr_list);
    return p_basic_block;
}

void ir_basic_block_set_br(p_ir_basic_block p_bb, p_ir_basic_block p_next) {
    p_bb->p_branch->kind = ir_br_branch;
    p_bb->p_branch->p_target_1 = ir_basic_block_branch_target_gen(p_next);
    ir_basic_block_add_prev(p_bb, p_next);
}
void ir_basic_block_set_cond(p_ir_basic_block p_bb, p_ir_operand p_exp, p_ir_basic_block p_true, p_ir_basic_block p_false) {
    p_bb->p_branch->kind = ir_cond_branch;
    p_bb->p_branch->p_exp = p_exp;
    p_bb->p_branch->p_target_1 = ir_basic_block_branch_target_gen(p_true);
    p_bb->p_branch->p_target_2 = ir_basic_block_branch_target_gen(p_false);
    ir_basic_block_add_prev(p_bb, p_true);
    ir_basic_block_add_prev(p_bb, p_false);
}
void ir_basic_block_set_ret(p_ir_basic_block p_bb, p_ir_operand p_exp) {
    p_bb->p_branch->kind = ir_ret_branch;
    p_bb->p_branch->p_exp = p_exp;
}

p_ir_basic_block_branch_target ir_basic_block_branch_target_gen(p_ir_basic_block p_block) {
    p_ir_basic_block_branch_target p_branch_target = malloc(sizeof(*p_branch_target));
    *p_branch_target = (ir_basic_block_branch_target) {
        .p_block = p_block,
        .p_block_param = ir_bb_param_list_init(),
    };
    return p_branch_target;
}

void ir_basic_block_branch_target_add_param(p_ir_basic_block_branch_target p_branch_target, p_ir_operand p_operand) {
    ir_bb_param_list_add(p_branch_target->p_block_param, p_operand);
}

void ir_basic_block_add_dom_son(p_ir_basic_block p_basic_block, p_ir_basic_block p_son) {
    p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (ir_basic_block_list_node) {
        .p_basic_block = p_son,
        .node = list_head_init(&p_new_node->node),
    };
    p_son->p_dom_parent = p_basic_block;
    list_add_prev(&p_new_node->node, &p_basic_block->dom_son_list);
}

void ir_basic_block_add_param(p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    ir_bb_phi_list_add(p_basic_block->basic_block_phis, p_vreg);
    ir_vreg_set_bb_def(p_vreg, p_basic_block);
}

void ir_basic_block_drop(p_ir_basic_block p_basic_block) {
    assert(p_basic_block);
    list_del(&p_basic_block->node);
    while (!list_head_alone(&p_basic_block->instr_list)) {
        p_ir_instr p_instr = list_entry(p_basic_block->instr_list.p_next, ir_instr, node);
        ir_instr_drop(p_instr);
    }
    while (!list_head_alone(&p_basic_block->prev_basic_block_list)) {
        p_ir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block->prev_basic_block_list.p_next, ir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        free(p_basic_block_list_node);
    }
    while (!list_head_alone(&p_basic_block->dom_son_list)) {
        p_ir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block->dom_son_list.p_next, ir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        free(p_basic_block_list_node);
    }
    ir_basic_block_branch_drop(p_basic_block->p_branch);
    ir_bb_phi_list_drop(p_basic_block->basic_block_phis);
    free(p_basic_block);
}

void ir_basic_block_branch_target_drop(p_ir_basic_block_branch_target p_branch_target) {
    ir_bb_param_list_drop(p_branch_target->p_block_param);
    free(p_branch_target);
}
