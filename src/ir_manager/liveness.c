#include <ir_gen.h>
#include <ir_manager/liveness.h>
#include <symbol_gen.h>

static inline void p_live_in_statments(p_liveness_info p_info, p_ir_instr p_instr, p_ir_vreg p_vreg);
static inline void p_live_out_statments(p_liveness_info p_info, p_ir_instr p_instr, p_ir_vreg p_vreg);
static inline void p_live_out_block(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
static inline void p_live_in_block(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
static inline void p_live_in_branch(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);

static inline void set_graph_table_edge(p_liveness_info p_info, p_ir_vreg r1, p_ir_vreg r2) {
    if (p_info->use_table)
        p_info->graph_table[r1->id][r2->id] = p_info->graph_table[r2->id][r1->id] = true;
    else {
        if (!if_in_vreg_list(p_info->graph_list[r1->id], r2)
            && !if_in_vreg_list(p_info->graph_list[r2->id], r1))
            ir_vreg_list_add(p_info->graph_list[r1->id], r2);
    }
}
static inline bool in_bb_phi_list(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->basic_block_phis) {
        p_ir_vreg p_phi = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        if (p_phi == p_vreg)
            return true;
        set_graph_table_edge(p_info, p_vreg, p_phi);
    }
    return false;
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
static inline void p_live_in_statments(p_liveness_info p_info, p_ir_instr p_instr, p_ir_vreg p_vreg) {
    p_ir_basic_block p_basic_block = p_instr->p_basic_block;
    while (1) {
        if (p_info->use_table)
            bitmap_add_element(p_info->instr_live_in[p_instr->instr_id], p_vreg->id);
        else {
            if (!if_in_vreg_list(p_instr->p_live_in, p_vreg))
                ir_vreg_list_add(p_instr->p_live_in, p_vreg);
        }
        if (&p_instr->node == p_basic_block->instr_list.p_next) {
            if (in_bb_phi_list(p_info, p_basic_block, p_vreg))
                return;
            p_live_in_block(p_info, p_basic_block, p_vreg);
            break;
        }
        else {
            p_ir_instr p_last_instr = list_entry(p_instr->node.p_prev, ir_instr, node);
            if (p_info->use_table)
                bitmap_add_element(p_info->instr_live_out[p_last_instr->instr_id], p_vreg->id);
            else {
                if (!if_in_vreg_list(p_last_instr->p_live_out, p_vreg))
                    ir_vreg_list_add(p_last_instr->p_live_out, p_vreg);
            }
            p_ir_vreg p_des = add_edge_with(p_last_instr);
            if (p_des) {
                if (p_des != p_vreg) {
                    set_graph_table_edge(p_info, p_vreg, p_des);
                    p_instr = p_last_instr;
                }
                else
                    break;
            }
            else
                p_instr = p_last_instr;
        }
    }
}

static inline void p_live_out_statments(p_liveness_info p_info, p_ir_instr p_instr, p_ir_vreg p_vreg) {
    if (p_info->use_table)
        bitmap_add_element(p_info->instr_live_out[p_instr->instr_id], p_vreg->id);
    else {
        if (!if_in_vreg_list(p_instr->p_live_out, p_vreg))
            ir_vreg_list_add(p_instr->p_live_out, p_vreg);
    }
    p_ir_vreg p_des = add_edge_with(p_instr);
    if (p_des) {
        if (p_des != p_vreg) {
            set_graph_table_edge(p_info, p_vreg, p_des);
            p_live_in_statments(p_info, p_instr, p_vreg);
        }
    }
    else
        p_live_in_statments(p_info, p_instr, p_vreg);
}

static inline void p_live_in_branch(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    if (p_info->use_table)
        bitmap_add_element(p_info->block_branch_live_in[p_basic_block->block_id], p_vreg->id);
    else {
        if (!if_in_vreg_list(p_basic_block->p_branch->p_live_in, p_vreg))
            ir_vreg_list_add(p_basic_block->p_branch->p_live_in, p_vreg);
    }
    if (list_head_alone(&p_basic_block->instr_list)) {
        if (in_bb_phi_list(p_info, p_basic_block, p_vreg))
            return;
        p_live_in_block(p_info, p_basic_block, p_vreg);
    }
    else {
        p_ir_instr p_last_instr = list_entry(p_basic_block->instr_list.p_prev, ir_instr, node);
        p_live_out_statments(p_info, p_last_instr, p_vreg);
    }
}

static inline void p_live_out_block(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    if (p_basic_block->if_visited)
        return;
    p_basic_block->if_visited = true;
    if (p_info->use_table)
        bitmap_add_element(p_info->block_live_out[p_basic_block->block_id], p_vreg->id);
    else {
        if (!if_in_vreg_list(p_basic_block->p_live_out, p_vreg))
            ir_vreg_list_add(p_basic_block->p_live_out, p_vreg);
    }
    p_live_in_branch(p_info, p_basic_block, p_vreg);
}

static inline void p_live_in_block(p_liveness_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    if (p_info->use_table)
        bitmap_add_element(p_info->block_live_in[p_basic_block->block_id], p_vreg->id);
    else {
        if (!if_in_vreg_list(p_basic_block->p_live_in, p_vreg))
            ir_vreg_list_add(p_basic_block->p_live_in, p_vreg);
    }
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->prev_branch_target_list) {
        p_ir_basic_block p_prev_block = list_entry(p_node, ir_branch_target_node, node)->p_target->p_source_block;
        p_live_out_block(p_info, p_prev_block, p_vreg);
    }
    if (p_info->p_func->p_entry_block == p_basic_block) {
        list_for_each(p_node, &p_info->p_func->param_reg_list) {
            p_ir_vreg p_param = list_entry(p_node, ir_vreg, node);
            if (p_param == p_vreg)
                return;
            set_graph_table_edge(p_info, p_param, p_vreg);
        }
        return;
    }
}

static inline void set_live(p_liveness_info p_info, p_ir_vreg_list p_list, p_bitmap p_b) {
    for (size_t i = 0; i < p_info->p_func->param_reg_cnt + p_info->p_func->vreg_cnt; i++) {
        if (bitmap_if_in(p_b, i))
            ir_vreg_list_add(p_list, p_info->p_vregs[i]);
    }
}

// 将 info中的 bitmap 转为链表
static inline void set_func_live(p_liveness_info p_info) {
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
    size_t vreg_num = p_func->vreg_cnt + p_func->param_reg_cnt;
    p_info->vreg_num = vreg_num;
    p_list_head p_node;
    p_info->p_vregs = malloc(sizeof(*p_info->p_vregs) * vreg_num);
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_info->p_vregs[p_vreg->id] = p_vreg;
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_info->p_vregs[p_vreg->id] = p_vreg;
    }
    if (p_func->vreg_cnt > 10000) {
        p_info->use_table = false;
        p_info->graph_list = malloc(sizeof(void *) * vreg_num);
        for (size_t i = 0; i < vreg_num; i++) {
            p_info->graph_list[i] = ir_vreg_list_init();
        }
        return p_info;
    }
    p_info->use_table = true;

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
    size_t instr_num = p_func->instr_num;
    p_info->instr_num = instr_num;
    p_info->instr_live_in = malloc(sizeof(void *) * instr_num);
    p_info->instr_live_out = malloc(sizeof(void *) * instr_num);
    for (size_t i = 0; i < instr_num; i++) {
        p_info->instr_live_in[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->instr_live_in[i]);
        p_info->instr_live_out[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->instr_live_out[i]);
    }
    p_info->graph_table = malloc(sizeof(*p_info->graph_table) * vreg_num);
    for (size_t i = 0; i < vreg_num; i++) {
        p_info->graph_table[i] = malloc(sizeof(*p_info->graph_table[i]) * vreg_num);
        memset(p_info->graph_table[i], false, sizeof(*p_info->graph_table[i]) * vreg_num);
    }
    return p_info;
}

void liveness_info_drop(p_liveness_info p_info) {
    free(p_info->p_vregs);
    if (p_info->use_table) {
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
    }
    else {
        for (size_t i = 0; i < p_info->vreg_num; i++) {
            ir_vreg_list_drop(p_info->graph_list[i]);
        }
        free(p_info->graph_list);
    }
    free(p_info);
}
static inline void deal_vreg(p_liveness_info p_info, p_ir_vreg p_vreg) {
    symbol_func_basic_block_init_visited(p_info->p_func);
    p_list_head p_use_node;
    list_for_each(p_use_node, &p_vreg->use_list) {
        p_ir_operand p_operand = list_entry(p_use_node, ir_operand, use_node);
        switch (p_operand->used_type) {
        case instr_ptr:
            p_live_in_statments(p_info, p_operand->p_instr, p_vreg);
            break;
        case bb_param_ptr:
            p_live_in_branch(p_info, p_operand->p_bb_param->p_target->p_source_block, p_vreg);
            break;
        case cond_ptr:
            break;
        case ret_ptr:
            p_live_in_branch(p_info, p_operand->p_basic_block, p_vreg);
            break;
        }
    }
}
static inline void liveness_set_clear(p_symbol_func p_func) {
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_block = list_entry(p_block_node, ir_basic_block, node);
        ir_vreg_list_clear(p_block->p_live_in);
        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            ir_vreg_list_clear(p_instr->p_live_in);
            ir_vreg_list_clear(p_instr->p_live_out);
        }
        ir_vreg_list_clear(p_block->p_branch->p_live_in);
        ir_vreg_list_clear(p_block->p_live_out);
    }
}
// 活跃性分析，如果用变量的使用列表效率可以更高
void liveness_analysis(p_liveness_info p_info) {
    liveness_set_clear(p_info->p_func);
    p_list_head p_vreg_node;
    list_for_each(p_vreg_node, &p_info->p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_vreg_node, ir_vreg, node);
        deal_vreg(p_info, p_vreg);
    }
    list_for_each(p_vreg_node, &p_info->p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_vreg_node, ir_vreg, node);
        deal_vreg(p_info, p_vreg);
    }
    if (p_info->use_table)
        set_func_live(p_info);
}
