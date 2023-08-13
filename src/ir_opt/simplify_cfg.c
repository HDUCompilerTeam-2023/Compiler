#include <ir_gen.h>
#include <ir_opt/simplify_cfg.h>
#include <program/def.h>
#include <stdio.h>
#include <symbol_gen/func.h>

static inline void ir_simplify_cfg_dfs_basic_block(p_ir_basic_block p_bb) {
    if (p_bb->if_visited) return;
    p_bb->if_visited = true;

    p_ir_basic_block_branch_target p_target_1 = p_bb->p_branch->p_target_1;
    p_ir_basic_block_branch_target p_target_2 = p_bb->p_branch->p_target_2;
    if (p_target_1) {
        ir_simplify_cfg_dfs_basic_block(p_target_1->p_block);
    }
    if (p_target_2) {
        ir_simplify_cfg_dfs_basic_block(p_target_2->p_block);
    }
}
static inline void ir_simplify_cfg_anti_dfs_basic_block(p_ir_basic_block p_bb) {
    if (p_bb->if_visited) return;
    p_bb->if_visited = true;
    p_list_head p_node;
    list_for_each(p_node, &p_bb->prev_branch_target_list) {
        p_ir_basic_block p_prev = list_entry(p_node, ir_branch_target_node, node)->p_target->p_source_block;
        ir_simplify_cfg_anti_dfs_basic_block(p_prev);
    }
}
static inline void ir_simplify_cfg_func_remove_no_predesessor_bb(p_symbol_func p_func) {
    symbol_func_basic_block_init_visited(p_func);
    p_ir_basic_block p_entry_bb = p_func->p_entry_block;
    ir_simplify_cfg_dfs_basic_block(p_entry_bb);

    p_ir_vreg *need_del = (p_ir_vreg *) malloc(sizeof(p_ir_vreg) * p_func->vreg_cnt);
    size_t del_reg_cnt = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if (p_bb->if_visited) continue;
        assert(p_bb != p_func->p_ret_block);
        assert(p_bb != p_func->p_entry_block);

        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_bb->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            p_ir_vreg p_des = NULL;
            switch (p_instr->irkind) {
            case ir_binary:
                p_des = p_instr->ir_binary.p_des;
                break;
            case ir_unary:
                p_des = p_instr->ir_unary.p_des;
                break;
            case ir_call:
                p_des = p_instr->ir_call.p_des;
                break;
            case ir_gep:
                p_des = p_instr->ir_gep.p_des;
                break;
            case ir_load:
                p_des = p_instr->ir_load.p_des;
            case ir_store:
                break;
            }
            if (p_des) {
                need_del[del_reg_cnt++] = p_des;
            }
        }
    }
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if (p_bb->if_visited) continue;

        p_list_head p_node;
        list_for_each(p_node, &p_bb->basic_block_phis) {
            p_ir_vreg p_del = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
            need_del[del_reg_cnt++] = p_del;
        }
        symbol_func_bb_del(p_func, p_bb);
    }
    for (size_t i = 0; i < del_reg_cnt; ++i) {
        symbol_func_vreg_del(p_func, need_del[i]);
    }
    free(need_del);
}
static inline void ir_simplify_cfg_func_remove_anti_no_predesessor_bb(p_symbol_func p_func) {
    symbol_func_basic_block_init_visited(p_func);
    ir_simplify_cfg_anti_dfs_basic_block(p_func->p_ret_block);

    p_ir_vreg *need_del = (p_ir_vreg *) malloc(sizeof(p_ir_vreg) * p_func->vreg_cnt);
    size_t del_reg_cnt = 0;

    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if (p_bb->if_visited) continue;
        assert(p_bb != p_func->p_ret_block);
        assert(p_bb != p_func->p_entry_block);

        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_bb->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            p_ir_vreg p_des = NULL;
            switch (p_instr->irkind) {
            case ir_binary:
                p_des = p_instr->ir_binary.p_des;
                break;
            case ir_unary:
                p_des = p_instr->ir_unary.p_des;
                break;
            case ir_call:
                p_des = p_instr->ir_call.p_des;
                break;
            case ir_gep:
                p_des = p_instr->ir_gep.p_des;
                break;
            case ir_load:
                p_des = p_instr->ir_load.p_des;
            case ir_store:
                break;
            }
            if (p_des) {
                need_del[del_reg_cnt++] = p_des;
            }
        }
    }
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if (p_bb->if_visited) continue;

        p_list_head p_node;
        list_for_each(p_node, &p_bb->basic_block_phis) {
            p_ir_vreg p_del = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
            need_del[del_reg_cnt++] = p_del;
        }
        symbol_func_bb_del(p_func, p_bb);
    }
    for (size_t i = 0; i < del_reg_cnt; ++i) {
        symbol_func_vreg_del(p_func, need_del[i]);
    }
    free(need_del);
}

static inline void ir_simplify_cfg_func_merge_single_predecessor_bb(p_symbol_func p_func) {
    p_ir_vreg *need_del = (p_ir_vreg *) malloc(sizeof(p_ir_vreg) * p_func->vreg_cnt);
    size_t del_reg_cnt = 0;
    p_list_head p_node;
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if ((&p_bb->prev_branch_target_list)->p_next->p_next != &p_bb->prev_branch_target_list) continue;
        if (p_bb == p_func->p_entry_block) continue;

        assert(!list_head_alone(&p_bb->prev_branch_target_list));

        p_ir_basic_block p_prev_bb = list_entry(p_bb->prev_branch_target_list.p_prev, ir_branch_target_node, node)->p_target->p_source_block;
        if (p_prev_bb->p_branch->kind != ir_br_branch) continue;
        assert(p_prev_bb != p_bb);

        p_list_head p_node, p_node_src = p_prev_bb->p_branch->p_target_1->block_param.p_next;
        list_for_each(p_node, &p_bb->basic_block_phis) {
            assert(p_node_src != &p_prev_bb->p_branch->p_target_1->block_param);
            p_ir_vreg p_des = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
            p_ir_operand p_src = list_entry(p_node_src, ir_bb_param, node)->p_bb_param;
            assert(p_src->kind == reg);
            p_list_head p_node, p_next;
            list_for_each_safe(p_node, p_next, &p_des->use_list) {
                p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
                assert(p_use->kind == reg);
                ir_operand_reset_vreg(p_use, p_src->p_vreg);
            }
            need_del[del_reg_cnt++] = p_des;
            p_node_src = p_node_src->p_next;
        }
        assert(p_node_src == &p_prev_bb->p_branch->p_target_1->block_param);

        ir_basic_block_add_instr_list(p_prev_bb, p_bb);
        p_bb->instr_list.p_next = &p_bb->instr_list;
        p_bb->instr_list.p_prev = &p_bb->instr_list;

        p_ir_basic_block_branch p_tmp_branch = p_prev_bb->p_branch;
        ir_basic_block_set_branch(p_prev_bb, p_bb->p_branch);
        ir_basic_block_set_branch(p_bb, p_tmp_branch);

        symbol_func_bb_del(p_func, p_bb);
    }
    for (size_t i = 0; i < del_reg_cnt; ++i) {
        symbol_func_vreg_del(p_func, need_del[i]);
    }
    free(need_del);
}

static inline bool ir_simplify_cfg_func_eliminate_single_br_bb(p_symbol_func p_func) {
    bool if_del = false;

    p_list_head p_node;
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if (!list_head_alone(&p_bb->instr_list)) continue;
        if (!list_head_alone(&p_bb->basic_block_phis)) continue;

        p_ir_basic_block_branch_target p_target = p_bb->p_branch->p_target_1;
        if (!p_target || p_bb->p_branch->p_target_2) continue;
        if (p_target->p_block == p_bb) continue;

        if (p_bb == p_func->p_entry_block && !list_head_alone(&p_target->block_param))
            continue;

        p_list_head p_node, p_next;

        list_for_each_safe(p_node, p_next, &p_bb->prev_branch_target_list) {
            p_ir_basic_block p_prev_bb = list_entry(p_node, ir_branch_target_node, node)->p_target->p_source_block;
            p_ir_basic_block_branch_target *pp_prev_target_del;
            p_ir_basic_block_branch_target p_prev_target_del;
            p_ir_basic_block_branch_target p_prev_target_no_del;
            if (p_prev_bb->p_branch->p_target_1->p_block == p_bb) {
                pp_prev_target_del = &p_prev_bb->p_branch->p_target_1;
                p_prev_target_del = p_prev_bb->p_branch->p_target_1;
                p_prev_target_no_del = p_prev_bb->p_branch->p_target_2;
            }
            else {
                assert(p_prev_bb->p_branch->p_target_2 && p_prev_bb->p_branch->p_target_2->p_block == p_bb);
                pp_prev_target_del = &p_prev_bb->p_branch->p_target_2;
                p_prev_target_del = p_prev_bb->p_branch->p_target_2;
                p_prev_target_no_del = p_prev_bb->p_branch->p_target_1;
            }

            p_ir_basic_block_branch_target p_prev_target_new = ir_basic_block_branch_target_gen(p_target->p_block);

            p_list_head p_node;
            list_for_each(p_node, &p_target->block_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                assert(p_param->kind == reg);
                p_ir_vreg p_param_reg = p_param->p_vreg;
                ir_basic_block_branch_target_add_param(p_prev_target_new, ir_operand_vreg_gen(p_param_reg));
            }
            ir_basic_block_branch_target_drop(p_prev_bb, p_prev_target_del);
            *pp_prev_target_del = p_prev_target_new;
            // it's dangerous
            p_prev_target_new->p_source_block = p_prev_bb;
            ir_basic_block_add_prev_target(p_prev_target_new, p_target->p_block);

            if (p_prev_target_no_del && p_prev_target_no_del->p_block == p_target->p_block) {
                p_list_head p_node_new, p_node_old = p_prev_target_no_del->block_param.p_next;
                list_for_each(p_node_new, &p_prev_target_new->block_param) {
                    assert(p_node_old != &p_prev_target_no_del->block_param);
                    p_ir_operand p_param_new = list_entry(p_node_new, ir_bb_param, node)->p_bb_param;
                    p_ir_operand p_param_old = list_entry(p_node_old, ir_bb_param, node)->p_bb_param;
                    assert(p_param_new->kind == reg);
                    assert(p_param_old->kind == reg);
                    if (p_param_new->p_vreg != p_param_old->p_vreg)
                        break;
                    p_node_old = p_node_old->p_next;
                }
                if (p_node_new == &p_prev_target_new->block_param) {
                    assert(p_node_old == &p_prev_target_no_del->block_param);
                    if_del = true;
                    p_prev_bb->p_branch->kind = ir_br_branch;
                    ir_operand_drop(p_prev_bb->p_branch->p_exp);
                    p_prev_bb->p_branch->p_exp = NULL;
                    ir_basic_block_branch_target_drop(p_prev_bb, p_prev_bb->p_branch->p_target_2);
                    p_prev_bb->p_branch->p_target_2 = NULL;
                }
            }
        }

        if (p_bb == p_func->p_entry_block)
            p_func->p_entry_block = p_target->p_block;

        symbol_func_bb_del(p_func, p_bb);
    }
    return if_del;
}

static inline void ir_simplify_cfg_func_eliminate_single_predecessor_phi(p_symbol_func p_func) {
    p_ir_vreg *need_del = (p_ir_vreg *) malloc(sizeof(p_ir_vreg) * p_func->vreg_cnt);
    size_t del_reg_cnt = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if ((&p_bb->prev_branch_target_list)->p_next->p_next != &p_bb->prev_branch_target_list) continue;
        if (p_bb == p_func->p_entry_block) continue;
        assert(!list_head_alone(&p_bb->prev_branch_target_list));
        if (list_head_alone(&p_bb->basic_block_phis))
            continue;

        p_ir_basic_block p_prev_bb = list_entry(p_bb->prev_branch_target_list.p_next, ir_branch_target_node, node)->p_target->p_source_block;
        p_ir_basic_block_branch_target p_target = p_prev_bb->p_branch->p_target_1;
        assert(p_target);
        if (p_target->p_block != p_bb)
            p_target = p_prev_bb->p_branch->p_target_2;
        assert(p_target);

        p_list_head p_node, p_node_src = p_target->block_param.p_next;
        list_for_each(p_node, &p_bb->basic_block_phis) {
            assert(p_node_src != &p_target->block_param);
            p_ir_vreg p_des = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
            p_ir_operand p_src = list_entry(p_node_src, ir_bb_param, node)->p_bb_param;
            assert(p_src->kind == reg);
            p_list_head p_node, p_next;
            list_for_each_safe(p_node, p_next, &p_des->use_list) {
                p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
                assert(p_use->kind == reg);
                ir_operand_reset_vreg(p_use, p_src->p_vreg);
            }
            need_del[del_reg_cnt++] = p_des;
            p_node_src = p_node_src->p_next;
        }
        assert(p_node_src == &p_target->block_param);

        ir_basic_block_clear_phi(p_bb);
        ir_basic_block_branch_target_clear_param(p_target);
    }
    for (size_t i = 0; i < del_reg_cnt; ++i) {
        symbol_func_vreg_del(p_func, need_del[i]);
    }
    free(need_del);
}

static inline bool ir_simplify_cfg_func_pass(p_symbol_func p_func) {
    assert(p_func->p_entry_block);

    bool if_del;
    ir_simplify_cfg_func_remove_no_predesessor_bb(p_func);
    ir_simplify_cfg_func_remove_anti_no_predesessor_bb(p_func);
    ir_simplify_cfg_func_eliminate_single_predecessor_phi(p_func);

    ir_simplify_cfg_func_merge_single_predecessor_bb(p_func);
    if_del = ir_simplify_cfg_func_eliminate_single_br_bb(p_func);
    return if_del;
}

bool ir_simplify_cfg_pass(p_program p_program) {
    bool if_del = false;
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        symbol_func_set_block_id(p_func);
        if_del |= ir_simplify_cfg_func_pass(p_func);
        symbol_func_set_block_id(p_func);
    }
    return if_del;
}
