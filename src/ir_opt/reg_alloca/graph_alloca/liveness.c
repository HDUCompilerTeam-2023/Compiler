#include <ir_opt/reg_alloca/graph_alloca/liveness.h>

#include <ir_gen.h>
#include <symbol_gen.h>

static void p_live_in_statments(p_liveness_info p_info, p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
static void p_live_out_statments(p_liveness_info p_info, p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
static void p_live_out_block(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
static void p_live_in_block(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
static void p_live_in_branch(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);

static inline void set_graph_table_edge(p_liveness_info p_info, p_ir_vreg r1, p_ir_vreg r2) {
    p_info->graph_table[r1->id][r2->id] = p_info->graph_table[r2->id][r1->id] = true;
}
static inline bool in_bb_phi_list(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->basic_block_phis->bb_phi) {
        p_ir_vreg p_phi = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        if (p_phi == p_vreg)
            return true;
        set_graph_table_edge(p_info, p_vreg, p_phi);
    }
    return false;
}

static void p_live_in_statments(p_liveness_info p_info, p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    bitmap_add_element(p_info->instr_live_in[p_instr->instr_id], p_vreg->id);
    if (&p_instr->node == p_basic_block->instr_list.p_next) {
        if (in_bb_phi_list(p_info, p_basic_block, p_vreg))
            return;
        p_live_in_block(p_info, p_basic_block, p_vreg);
    }
    else {
        p_ir_instr p_last_instr = list_entry(p_instr->node.p_prev, ir_instr, node);
        p_live_out_statments(p_info, p_last_instr, p_basic_block, p_vreg);
    }
}
static inline void p_live_in_statments_(p_liveness_info p_info, p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_ir_operand p_operand) {
    if (p_operand->kind == reg) {
        symbol_func_basic_block_init_visited(p_info->p_func);
        p_live_in_statments(p_info, p_instr, p_basic_block, p_operand->p_vreg);
    }
}

static inline p_ir_vreg add_edge_with(p_ir_instr p_instr) {
    switch (p_instr->irkind) {
    case ir_binary:
        switch (p_instr->ir_binary.op) {
        case ir_add_op:
        case ir_sub_op:
        case ir_mul_op:
        case ir_div_op:
            return p_instr->ir_binary.p_des;
        case ir_eq_op:
        case ir_neq_op:
        case ir_g_op:
        case ir_geq_op:
        case ir_l_op:
        case ir_leq_op:
            if (p_instr->ir_binary.p_des->if_cond)
                return NULL;
            return p_instr->ir_binary.p_des;
        case ir_mod_op:
            assert(0);
            break;
        }
    case ir_call:
        return p_instr->ir_call.p_des;
    case ir_unary:
        return p_instr->ir_unary.p_des;
    case ir_load:
        return p_instr->ir_load.p_des;
    case ir_store:
        return NULL;
    case ir_gep:
        assert(0);
    }
}

static void p_live_out_statments(p_liveness_info p_info, p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    bitmap_add_element(p_info->instr_live_out[p_instr->instr_id], p_vreg->id);
    p_ir_vreg p_des = add_edge_with(p_instr);
    if (p_des) {
        if (p_des != p_vreg) {
            set_graph_table_edge(p_info, p_vreg, p_des);
            p_live_in_statments(p_info, p_instr, p_basic_block, p_vreg);
        }
    }
    else
        p_live_in_statments(p_info, p_instr, p_basic_block, p_vreg);
}

static void p_live_in_branch(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    bitmap_add_element(p_info->block_branch_live_in[p_basic_block->block_id], p_vreg->id);
    if (list_head_alone(&p_basic_block->instr_list)) {
        if (in_bb_phi_list(p_info, p_basic_block, p_vreg))
            return;
        p_live_in_block(p_info, p_basic_block, p_vreg);
    }
    else {
        p_ir_instr p_last_instr = list_entry(p_basic_block->instr_list.p_prev, ir_instr, node);
        p_live_out_statments(p_info, p_last_instr, p_basic_block, p_vreg);
    }
}

static void p_live_in_branch_(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_operand p_operand) {
    if (p_operand->kind == reg) {
        symbol_func_basic_block_init_visited(p_info->p_func);
        p_live_in_branch(p_info, p_basic_block, p_operand->p_vreg);
    }
}

static void p_live_out_block(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    if (p_basic_block->if_visited)
        return;
    p_basic_block->if_visited = true;
    bitmap_add_element(p_info->block_live_out[p_basic_block->block_id], p_vreg->id);
    p_live_in_branch(p_info, p_basic_block, p_vreg);
}

void p_live_in_block(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    bitmap_add_element(p_info->block_live_in[p_basic_block->block_id], p_vreg->id);
    p_list_head p_node;
    if (p_info->p_func->block.p_next == &p_basic_block->node) {
        list_for_each(p_node, &p_info->p_func->param_reg_list) {
            p_ir_vreg p_param = list_entry(p_node, ir_vreg, node);
            if (p_param == p_vreg)
                return;
            set_graph_table_edge(p_info, p_param, p_vreg);
        }
        return;
    }
    list_for_each(p_node, &p_basic_block->prev_basic_block_list) {
        p_ir_basic_block p_prev_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        p_live_out_block(p_info, p_prev_block, p_vreg);
    }
}

static inline void set_live(p_liveness_info p_info, p_ir_bb_phi_list p_list, p_bitmap p_b) {
    for (size_t i = 0; i < p_info->p_func->param_reg_cnt + p_info->p_func->vreg_cnt; i++) {
        if (bitmap_if_in(p_b, i))
            ir_bb_phi_list_add(p_list, p_info->p_vregs[i]);
    }
}

// 将 info中的 bitmap 转为链表
static void set_func_live(p_liveness_info p_info) {
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_info->p_func->block) {
        p_ir_basic_block p_block = list_entry(p_block_node, ir_basic_block, node);
        set_live(p_info, p_block->p_live_in, p_info->block_live_in[p_block->block_id]);
        set_live(p_info, p_block->p_live_out, p_info->block_live_out[p_block->block_id]);
        set_live(p_info, p_block->p_branch->p_live_in, p_info->block_branch_live_in[p_block->block_id]);
        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            set_live(p_info, p_instr->p_live_in, p_info->instr_live_in[p_instr->instr_id]);
            set_live(p_info, p_instr->p_live_out, p_info->instr_live_out[p_instr->instr_id]);
        }
    }
}

p_liveness_info liveness_info_gen(p_symbol_func p_func) {
    p_liveness_info p_info = malloc(sizeof(*p_info));
    p_info->p_func = p_func;
    p_list_head p_node;

    size_t vreg_num = p_func->vreg_cnt + p_func->param_reg_cnt;
    p_info->vreg_num = vreg_num;
    p_info->block_live_in = malloc(sizeof(void *) * p_func->block_cnt);
    p_info->block_live_out = malloc(sizeof(void *) * p_func->block_cnt);
    p_info->block_branch_live_in = malloc(sizeof(void *) * p_func->block_cnt);
    for (size_t i = 0; i < p_func->block_cnt; i++) {
        p_info->block_live_in[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->block_live_in[i]);
        p_info->block_live_out[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->block_live_out[i]);
        p_info->block_branch_live_in[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->block_branch_live_in[i]);
    }
    p_ir_basic_block p_last_block = list_entry(p_func->block.p_prev, ir_basic_block, node);
    while (list_head_alone(&p_last_block->instr_list))
        p_last_block = list_entry(p_last_block->node.p_prev, ir_basic_block, node);
    size_t instr_num = list_entry(p_last_block->instr_list.p_prev, ir_instr, node)->instr_id + 1;
    p_info->instr_live_in = malloc(sizeof(void *) * instr_num);
    p_info->instr_live_out = malloc(sizeof(void *) * instr_num);
    p_info->instr_num = instr_num;
    for (size_t i = 0; i < instr_num; i++) {
        p_info->instr_live_in[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->instr_live_in[i]);
        p_info->instr_live_out[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->instr_live_out[i]);
    }
    p_info->p_vregs = malloc(sizeof(*p_info->p_vregs) * vreg_num);
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_info->p_vregs[p_vreg->id] = p_vreg;
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_info->p_vregs[p_vreg->id] = p_vreg;
    }
    p_info->graph_table = malloc(sizeof(*p_info->graph_table) * vreg_num);
    for (size_t i = 0; i < vreg_num; i++) {
        p_info->graph_table[i] = malloc(sizeof(*p_info->graph_table[i]) * vreg_num);
        memset(p_info->graph_table[i], false, sizeof(*p_info->graph_table[i]) * vreg_num);
    }
    return p_info;
}

void liveness_info_drop(p_liveness_info p_info) {
    for (size_t i = 0; i < p_info->p_func->block_cnt; i++) {
        bitmap_drop(p_info->block_live_in[i]);
        bitmap_drop(p_info->block_live_out[i]);
        bitmap_drop(p_info->block_branch_live_in[i]);
    }

    for (size_t i = 0; i < p_info->instr_num; i++) {
        bitmap_drop(p_info->instr_live_in[i]);
        bitmap_drop(p_info->instr_live_out[i]);
    }

    for (size_t i = 0; i < p_info->vreg_num; i++)
        free(p_info->graph_table[i]);
    free(p_info->graph_table);
    free(p_info->block_live_in);
    free(p_info->block_live_out);
    free(p_info->block_branch_live_in);
    free(p_info->instr_live_in);
    free(p_info->instr_live_out);
    free(p_info->p_vregs);
    free(p_info);
}
// 活跃性分析，如果用变量的使用列表效率可以更高
void liveness_analysis(p_liveness_info p_info) {
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_info->p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_list_head p_instr_node;
        p_list_head p_node;
        list_for_each(p_instr_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            switch (p_instr->irkind) {
            case ir_unary:
                p_live_in_statments_(p_info, p_instr, p_basic_block, p_instr->ir_unary.p_src);
                break;
            case ir_binary:
                p_live_in_statments_(p_info, p_instr, p_basic_block, p_instr->ir_binary.p_src1);
                p_live_in_statments_(p_info, p_instr, p_basic_block, p_instr->ir_binary.p_src2);
                break;
            case ir_load:
                p_live_in_statments_(p_info, p_instr, p_basic_block, p_instr->ir_load.p_addr);
                break;
            case ir_store:
                p_live_in_statments_(p_info, p_instr, p_basic_block, p_instr->ir_store.p_addr);
                p_live_in_statments_(p_info, p_instr, p_basic_block, p_instr->ir_store.p_src);
                break;
            case ir_call:
                list_for_each(p_node, &p_instr->ir_call.p_param_list->param) {
                    p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
                    p_live_in_statments_(p_info, p_instr, p_basic_block, p_param);
                }
                break;
            case ir_gep:
                assert(0);
            }
        }
        switch (p_basic_block->p_branch->kind) {
        case ir_br_branch:
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                p_live_in_branch_(p_info, p_basic_block, p_param);
            }
            break;
        case ir_cond_branch:
            // 条件是不活跃的
            // p_live_out_statments_(p_info, p_last_instr, p_basic_block, p_basic_block->p_branch->p_exp);
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                p_live_in_branch_(p_info, p_basic_block, p_param);
            }
            list_for_each(p_node, &p_basic_block->p_branch->p_target_2->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                p_live_in_branch_(p_info, p_basic_block, p_param);
            }
            break;
        case ir_ret_branch:
            if (p_basic_block->p_branch->p_exp)
                p_live_in_branch_(p_info, p_basic_block, p_basic_block->p_branch->p_exp);
            break;
        case ir_abort_branch:
            break;
        }
    }
    set_func_live(p_info);
}
