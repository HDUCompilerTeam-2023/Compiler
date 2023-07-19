#include <ir_gen.h>
#include <ir_gen/basic_block.h>
#include <symbol_gen.h>
static inline p_ir_bb_param ir_bb_param_gen(p_ir_operand p_operand) {
    p_ir_bb_param p_ir_bb_param = malloc(sizeof(*p_ir_bb_param));
    *p_ir_bb_param = (ir_bb_param) {
        .node = list_head_init(&p_ir_bb_param->node),
    };
    if (p_operand) {
        p_ir_bb_param->p_bb_param = p_operand;
        p_operand->used_type = bb_param_ptr;
        p_operand->p_bb_param = p_ir_bb_param;
    }
    return p_ir_bb_param;
}
static inline void ir_bb_param_drop(p_ir_bb_param p_param) {
    if (p_param->p_bb_param) {
        assert(p_param->p_bb_param->used_type == bb_param_ptr);
        assert(p_param->p_bb_param->p_bb_param == p_param);
        ir_operand_drop(p_param->p_bb_param);
    }
    list_del(&p_param->node);
    free(p_param);
}
static inline void ir_vreg_set_bb_def(p_ir_vreg p_vreg, p_ir_bb_phi p_bb_phi) {
    p_vreg->def_type = bb_phi_def;
    p_vreg->p_bb_phi = p_bb_phi;
}
static inline p_ir_bb_phi ir_bb_phi_gen(p_ir_vreg p_vreg) {
    p_ir_bb_phi p_bb_phi = malloc(sizeof(*p_bb_phi));
    *p_bb_phi = (ir_bb_phi) {
        .node = list_head_init(&p_bb_phi->node),
    };
    ir_bb_phi_set_vreg(p_bb_phi, p_vreg);
    return p_bb_phi;
}
static inline void ir_bb_phi_drop(p_ir_bb_phi p_bb_phi) {
    assert(p_bb_phi->p_bb_phi->def_type == bb_phi_def);
    assert(p_bb_phi->p_bb_phi->p_bb_phi == p_bb_phi);
    list_del(&p_bb_phi->node);
    free(p_bb_phi);
}
static inline p_ir_basic_block_branch ir_basic_block_branch_gen() {
    p_ir_basic_block_branch p_branch = malloc(sizeof(*p_branch));
    *p_branch = (ir_basic_block_branch) {
        .kind = ir_abort_branch,
        .p_exp = NULL,
        .p_target_1 = NULL,
        .p_target_2 = NULL,
        .p_live_in = ir_vreg_list_init(),
    };
    return p_branch;
}
static inline void ir_basic_block_branch_drop(p_ir_basic_block p_source_block, p_ir_basic_block_branch p_branch) {
    assert(p_branch);
    if (p_branch->kind == ir_ret_branch && p_branch->p_exp) {
        assert(p_branch->p_exp->used_type == ret_ptr);
        assert(p_branch->p_exp->p_basic_block == p_source_block);
        ir_operand_drop(p_branch->p_exp);
    }
    if (p_branch->kind == ir_br_branch)
        ir_basic_block_branch_target_drop(p_source_block, p_branch->p_target_1);
    if (p_branch->kind == ir_cond_branch) {
        assert(p_branch->p_exp->used_type == cond_ptr);
        assert(p_branch->p_exp->p_basic_block == p_source_block);
        ir_operand_drop(p_branch->p_exp);
        ir_basic_block_branch_target_drop(p_source_block, p_branch->p_target_1);
        ir_basic_block_branch_target_drop(p_source_block, p_branch->p_target_2);
    }
    ir_vreg_list_drop(p_branch->p_live_in);
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
        .basic_block_phis = list_head_init(&p_ir_block->basic_block_phis),
        .dom_son_list = ir_basic_block_list_init(),
        .p_dom_parent = NULL,
        .dom_depth = 0,
        .if_visited = false,
        .p_live_in = ir_vreg_list_init(),
        .p_live_out = ir_vreg_list_init(),
        .instr_num = 0,
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
void ir_basic_block_insert_prev(p_ir_basic_block p_prev, p_ir_basic_block p_next) {
    if (p_next->p_func->p_entry_block == p_next) {
        assert(p_next == list_entry(p_next->p_func->block.p_next, ir_basic_block, node));
        p_next->p_func->p_entry_block = p_prev;
    }
    list_add_prev(&p_prev->node, &p_next->node);
    p_prev->p_func = p_next->p_func;
    p_prev->p_func->block_cnt++;
    p_prev->p_func->if_updated_graph = true;
}
void ir_basic_block_insert_next(p_ir_basic_block p_next, p_ir_basic_block p_prev) {
    assert(p_prev->p_func->p_ret_block != p_prev);
    if (p_next->p_branch->kind == ir_ret_branch)
        p_prev->p_func->p_ret_block = p_next;
    list_add_next(&p_next->node, &p_prev->node);
    p_next->p_func = p_prev->p_func;
    p_next->p_func->block_cnt++;
    p_next->p_func->if_updated_graph = true;
}
p_ir_basic_block ir_basic_block_addinstr_tail(p_ir_basic_block p_basic_block, p_ir_instr p_instr) {
    p_instr->p_basic_block = p_basic_block;
    list_add_prev(&p_instr->node, &p_basic_block->instr_list);
    p_basic_block->instr_num++;
    p_basic_block->p_func->instr_num++;
    return p_basic_block;
}
p_ir_basic_block ir_basic_block_addinstr_head(p_ir_basic_block p_basic_block, p_ir_instr p_instr) {
    p_instr->p_basic_block = p_basic_block;
    list_add_next(&p_instr->node, &p_basic_block->instr_list);
    p_basic_block->instr_num++;
    p_basic_block->p_func->instr_num++;
    return p_basic_block;
}
void ir_basic_block_add_instr_list(p_ir_basic_block p_des_block, p_ir_basic_block p_src_block) {
    p_list_head p_node, p_node_next;
    list_for_each_safe(p_node, p_node_next, &p_src_block->instr_list) {
        p_ir_instr p_src_instr = list_entry(p_node, ir_instr, node);
        assert(p_src_instr->p_basic_block == p_src_block);
        ir_instr_del(p_src_instr);
        ir_basic_block_addinstr_tail(p_des_block, p_src_instr);
    }
}

void ir_basic_block_set_target1(p_ir_basic_block p_basic_block, p_ir_basic_block_branch_target p_target) {
    p_basic_block->p_branch->p_target_1 = p_target;
    p_target->p_source_block = p_basic_block;
    p_basic_block->p_func->if_updated_graph = true;
}
void ir_basic_block_set_target2(p_ir_basic_block p_basic_block, p_ir_basic_block_branch_target p_target) {
    p_basic_block->p_branch->p_target_2 = p_target;
    p_target->p_source_block = p_basic_block;
    p_basic_block->p_func->if_updated_graph = true;
}
void ir_basic_block_set_br(p_ir_basic_block p_bb, p_ir_basic_block p_next) {
    p_bb->p_branch->kind = ir_br_branch;
    ir_basic_block_set_target1(p_bb, ir_basic_block_branch_target_gen(p_next));
    ir_basic_block_add_prev(p_bb, p_next);
}

void ir_basic_block_set_cond_exp(p_ir_basic_block p_basic_block, p_ir_operand p_exp) {
    assert(p_basic_block->p_branch->kind == ir_cond_branch);
    p_basic_block->p_branch->p_exp = p_exp;
    p_exp->used_type = cond_ptr;
    p_exp->p_basic_block = p_basic_block;
}
void ir_basic_block_set_cond(p_ir_basic_block p_bb, p_ir_operand p_exp, p_ir_basic_block p_true, p_ir_basic_block p_false) {
    p_bb->p_branch->kind = ir_cond_branch;
    ir_basic_block_set_target1(p_bb, ir_basic_block_branch_target_gen(p_true));
    ir_basic_block_set_target2(p_bb, ir_basic_block_branch_target_gen(p_false));
    ir_basic_block_set_cond_exp(p_bb, p_exp);
    ir_basic_block_add_prev(p_bb, p_true);
    ir_basic_block_add_prev(p_bb, p_false);
}
void ir_basic_block_set_branch(p_ir_basic_block p_basic_block, p_ir_basic_block_branch p_branch) {
    p_basic_block->p_branch = p_branch;
    if (p_branch->p_exp)
        p_branch->p_exp->p_basic_block = p_basic_block;
    if (p_branch->p_target_1)
        ir_basic_block_set_target1(p_basic_block, p_branch->p_target_1);
    if (p_branch->p_target_2)
        ir_basic_block_set_target2(p_basic_block, p_branch->p_target_2);
    if (p_branch->kind == ir_ret_branch)
        p_basic_block->p_func->p_ret_block = p_basic_block;
}

void ir_basic_block_set_ret(p_ir_basic_block p_bb, p_ir_operand p_exp) {
    p_bb->p_branch->kind = ir_ret_branch;
    p_bb->p_branch->p_exp = p_exp;
    p_bb->p_func->p_ret_block = p_bb;
    if (p_exp) {
        p_exp->used_type = ret_ptr;
        p_exp->p_basic_block = p_bb;
    }
}
void ir_basic_block_branch_target_clear_param(p_ir_basic_block_branch_target p_target) {
    while (!list_head_alone(&p_target->block_param)) {
        p_ir_bb_param p_bb_param = list_entry(p_target->block_param.p_next, ir_bb_param, node);
        ir_basic_block_branch_target_del_param(p_target, p_bb_param);
    }
}

p_ir_basic_block_branch_target ir_basic_block_branch_target_gen(p_ir_basic_block p_target_block) {
    p_ir_basic_block_branch_target p_branch_target = malloc(sizeof(*p_branch_target));
    *p_branch_target = (ir_basic_block_branch_target) {
        .block_param = list_head_init(&p_branch_target->block_param),
        .p_block = p_target_block,
    };
    return p_branch_target;
}
void ir_basic_block_branch_target_add_param(p_ir_basic_block_branch_target p_branch_target, p_ir_operand p_operand) {
    p_ir_bb_param p_param = ir_bb_param_gen(p_operand);
    p_param->p_target = p_branch_target;
    list_add_prev(&p_param->node, &p_branch_target->block_param);
}
void ir_basic_block_branch_target_del_param(p_ir_basic_block_branch_target p_branch_target, p_ir_bb_param p_param) {
    assert(p_param->p_target == p_branch_target);
    ir_bb_param_drop(p_param);
}
void ir_basic_block_add_dom_son(p_ir_basic_block p_basic_block, p_ir_basic_block p_son) {
    ir_basic_block_list_add(p_basic_block->dom_son_list, p_son);
    p_son->p_dom_parent = p_basic_block;
}
void ir_basic_block_add_phi(p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    p_ir_bb_phi p_ir_bb_phi = ir_bb_phi_gen(p_vreg);
    p_ir_bb_phi->p_basic_block = p_basic_block;
    list_add_prev(&p_ir_bb_phi->node, &p_basic_block->basic_block_phis);
    ir_vreg_set_bb_def(p_vreg, p_ir_bb_phi);
}

void ir_basic_block_del_phi(p_ir_basic_block p_basic_block, p_ir_bb_phi p_bb_phi) {
    assert(p_bb_phi->p_basic_block == p_basic_block);
    ir_bb_phi_drop(p_bb_phi);
}
void ir_basic_block_clear_phi(p_ir_basic_block p_basic_block) {
    while (!list_head_alone(&p_basic_block->basic_block_phis)) {
        p_ir_bb_phi p_bb_phi = list_entry(p_basic_block->basic_block_phis.p_next, ir_bb_phi, node);
        assert(p_bb_phi->p_basic_block == p_basic_block);
        ir_basic_block_del_phi(p_basic_block, p_bb_phi);
    }
}

void ir_basic_block_drop(p_ir_basic_block p_basic_block) {
    assert(p_basic_block);
    list_del(&p_basic_block->node);
    while (!list_head_alone(&p_basic_block->instr_list)) {
        p_ir_instr p_instr = list_entry(p_basic_block->instr_list.p_next, ir_instr, node);
        assert(p_instr->p_basic_block == p_basic_block);
        ir_instr_drop(p_instr);
    }
    while (!list_head_alone(&p_basic_block->prev_basic_block_list)) {
        p_ir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block->prev_basic_block_list.p_next, ir_basic_block_list_node, node);
        ir_basic_block_list_node_drop(p_basic_block_list_node);
    }
    ir_basic_block_list_drop(p_basic_block->dom_son_list);
    ir_basic_block_branch_drop(p_basic_block, p_basic_block->p_branch);
    ir_basic_block_clear_phi(p_basic_block);
    ir_vreg_list_drop(p_basic_block->p_live_in);
    ir_vreg_list_drop(p_basic_block->p_live_out);
    p_basic_block->p_func->block_cnt--;
    assert(p_basic_block->instr_num == 0);
    free(p_basic_block);
}

void ir_basic_block_branch_target_drop(p_ir_basic_block p_source_block, p_ir_basic_block_branch_target p_branch_target) {
    assert(p_branch_target->p_source_block == p_source_block);
    ir_basic_block_branch_target_clear_param(p_branch_target);
    free(p_branch_target);
}

p_ir_basic_block_list ir_basic_block_list_init() {
    p_ir_basic_block_list p_block_list = malloc(sizeof(*p_block_list));
    p_block_list->block_list = list_head_init(&p_block_list->block_list);
    return p_block_list;
}
void ir_basic_block_list_clear(p_ir_basic_block_list p_block_list) {
    p_list_head p_node, p_node_next;
    list_for_each_safe(p_node, p_node_next, &p_block_list->block_list){
        p_ir_basic_block_list_node p_block_node = list_entry(p_node, ir_basic_block_list_node, node);
        ir_basic_block_list_node_drop(p_block_node);
    }
}
void ir_basic_block_list_add(p_ir_basic_block_list p_list, p_ir_basic_block p_basic_block) {
    p_ir_basic_block_list_node p_node = ir_basic_block_list_node_gen(p_basic_block);
    list_add_prev(&p_node->node, &p_list->block_list);
}
void ir_basic_block_list_drop(p_ir_basic_block_list p_basic_block_list) {
    ir_basic_block_list_clear(p_basic_block_list);
    free(p_basic_block_list);
}

void ir_basic_block_list_node_set_basic_block(p_ir_basic_block_list_node p_basic_block_list_node, p_ir_basic_block p_block) {
    p_basic_block_list_node->p_basic_block = p_block;
}
p_ir_basic_block_list_node ir_basic_block_list_node_gen(p_ir_basic_block p_basic_block) {
    p_ir_basic_block_list_node p_node = malloc(sizeof(*p_node));
    p_node->node = list_head_init(&p_node->node);
    p_node->p_basic_block = p_basic_block;
    return p_node;
}
void ir_basic_block_list_node_drop(p_ir_basic_block_list_node p_basic_block_list_node) {
    list_del(&p_basic_block_list_node->node);
    free(p_basic_block_list_node);
}

void copy_basic_block_list(p_ir_basic_block_list p_des, p_ir_basic_block_list p_src) {
    assert(list_head_alone(&p_des->block_list));
    p_list_head p_node;
    list_for_each(p_node, &p_src->block_list) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        ir_basic_block_list_add(p_des, p_basic_block);
    }
}

bool if_in_basic_block_list(p_ir_basic_block_list p_list, p_ir_basic_block p_block) {
    p_list_head p_node;
    list_for_each(p_node, &p_list->block_list) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        if (p_basic_block == p_block)
            return true;
    }
    return false;
}

void ir_basic_block_list_del(p_ir_basic_block_list p_list, p_ir_basic_block p_block) {
    p_list_head p_node;
    list_for_each(p_node, &p_list->block_list) {
        p_ir_basic_block_list_node p_basic_block_node = list_entry(p_node, ir_basic_block_list_node, node);
        if (p_basic_block_node->p_basic_block == p_block) {
            ir_basic_block_list_node_drop(p_basic_block_node);
            break;
        }
    }
}