#include <ir_opt/reg_alloca/graph_alloca/liveness.h>

#include <ir_gen.h>
#include <symbol_gen.h>

static void p_live_in_statments(p_graph_alloca_info p_info, p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
static void p_live_out_statments(p_graph_alloca_info p_info, p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
static void p_live_out_block(p_graph_alloca_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
static void p_live_in_block(p_graph_alloca_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);
static void p_live_in_branch(p_graph_alloca_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg);

static inline bool in_bb_phi_list(p_graph_alloca_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->basic_block_phis->bb_phi) {
        p_ir_vreg p_phi = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        if (p_phi == p_vreg)
            return true;
        add_reg_graph_edge(p_phi, p_vreg);
    }
    return false;
}

static void p_live_in_statments(p_graph_alloca_info p_info, p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
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
static inline void p_live_in_statments_(p_graph_alloca_info p_info, p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_ir_operand p_operand) {
    if (p_operand->kind == reg) {
        symbol_func_basic_block_init_visited(p_info->p_func);
        p_live_in_statments(p_info, p_instr, p_basic_block, p_operand->p_vreg);
    }
}

static void p_live_out_statments(p_graph_alloca_info p_info, p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    bitmap_add_element(p_info->instr_live_out[p_instr->instr_id], p_vreg->id);
    p_ir_vreg p_des = ir_instr_get_des(p_instr);
    if (p_des) {
        if (p_des != p_vreg) {
            add_reg_graph_edge(p_vreg, p_des);
            p_live_in_statments(p_info, p_instr, p_basic_block, p_vreg);
        }
    }
    else
        p_live_in_statments(p_info, p_instr, p_basic_block, p_vreg);
}

static void p_live_in_branch(p_graph_alloca_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
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

static void p_live_in_branch_(p_graph_alloca_info p_info, p_ir_basic_block p_basic_block, p_ir_operand p_operand) {
    if (p_operand->kind == reg) {
        symbol_func_basic_block_init_visited(p_info->p_func);
        p_live_in_branch(p_info, p_basic_block, p_operand->p_vreg);
    }
}

static void p_live_out_block(p_graph_alloca_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    if (p_basic_block->if_visited)
        return;
    p_basic_block->if_visited = true;
    bitmap_add_element(p_info->block_live_out[p_basic_block->block_id], p_vreg->id);
    p_live_in_branch(p_info, p_basic_block, p_vreg);
}

void p_live_in_block(p_graph_alloca_info p_info, p_ir_basic_block p_basic_block, p_ir_vreg p_vreg) {
    bitmap_add_element(p_info->block_live_in[p_basic_block->block_id], p_vreg->id);
    p_list_head p_node;
    if(p_info->p_func->block.p_next == &p_basic_block->node){
        list_for_each(p_node, &p_info->p_func->param_reg_list){
            p_ir_vreg p_param = list_entry(p_node, ir_vreg, node);
            if(p_param == p_vreg)
                return;
            add_reg_graph_edge(p_param, p_vreg);
        }
        return;
    }
    list_for_each(p_node, &p_basic_block->prev_basic_block_list) {
        p_ir_basic_block p_prev_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        p_live_out_block(p_info, p_prev_block, p_vreg);
    }
}

static inline void set_live(p_graph_alloca_info p_info, p_ir_bb_phi_list p_list, p_bitmap p_b) {
    for (size_t i = 0; i < p_info->p_func->param_reg_cnt + p_info->p_func->vreg_cnt; i++) {
        if (bitmap_if_in(p_b, i))
            ir_bb_phi_list_add(p_list, p_info->p_vregs[i]);
    }
}

bool if_in_live_set(p_ir_bb_phi_list p_list, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_list->bb_phi) {
        p_ir_vreg p_live = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        if (p_live == p_vreg)
            return true;
    }
    return false;
}

// 检验基本块参数传入的参数在跳转目标基本块的入口是不活跃的，避免错误，可删
void check_liveness(p_symbol_func p_func) {
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_list_head p_node;
        switch (p_basic_block->p_branch->kind) {
        case ir_br_branch:
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                if (p_param->kind == reg)
                    assert(!if_in_live_set(p_basic_block->p_branch->p_target_1->p_block->p_live_in, p_param->p_vreg));
            }
            break;
        case ir_cond_branch:
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                if (p_param->kind == reg)
                    assert(!if_in_live_set(p_basic_block->p_branch->p_target_1->p_block->p_live_in, p_param->p_vreg));
            }
            list_for_each(p_node, &p_basic_block->p_branch->p_target_2->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                if (p_param->kind == reg)
                    assert(!if_in_live_set(p_basic_block->p_branch->p_target_2->p_block->p_live_in, p_param->p_vreg));
            }
            break;
        case ir_ret_branch:
        case ir_abort_branch:
            break;
        }
    }
}

// 将 info中的 bitmap 转为链表
void set_func_live(p_graph_alloca_info p_info, p_symbol_func p_func) {
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
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

// 活跃性分析，如果用变量的使用列表效率可以更高
void liveness_analysis(p_graph_alloca_info p_info, p_symbol_func p_func) {
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
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
                if (p_instr->ir_load.p_offset)
                    p_live_in_statments_(p_info, p_instr, p_basic_block, p_instr->ir_load.p_offset);
                break;
            case ir_store:
                p_live_in_statments_(p_info, p_instr, p_basic_block, p_instr->ir_store.p_addr);
                p_live_in_statments_(p_info, p_instr, p_basic_block, p_instr->ir_store.p_src);
                if (p_instr->ir_store.p_offset)
                    p_live_in_statments_(p_info, p_instr, p_basic_block, p_instr->ir_store.p_offset);
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
}
