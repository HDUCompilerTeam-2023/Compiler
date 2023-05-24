#include <mir_gen.h>
#include <program/def.h>
#include <symbol/sym.h>
#include <mir_opt/simplify_cfg.h>
#include <stdio.h>

static inline void mir_simplify_cfg_dfs_basic_block(p_mir_basic_block p_bb) {
    if (p_bb->if_visited) return;
    p_bb->if_visited = true;

    p_mir_basic_block_branch_target p_target_1 = p_bb->p_branch->p_target_1;
    p_mir_basic_block_branch_target p_target_2 = p_bb->p_branch->p_target_2;
    if (p_target_1) {
        mir_simplify_cfg_dfs_basic_block(p_target_1->p_block);
    }
    if (p_target_2) {
        mir_simplify_cfg_dfs_basic_block(p_target_2->p_block);
    }
}

static inline void mir_simplify_cfg_func_remove_no_predesessor_bb(p_mir_func p_func) {
    mir_basic_block_init_visited(p_func);
    p_mir_basic_block p_entry_bb = list_entry(p_func->entry_block.p_next, mir_basic_block, node);
    mir_simplify_cfg_dfs_basic_block(p_entry_bb);
    p_list_head p_node;
    list_for_each(p_node, &p_func->entry_block) {
        p_mir_basic_block p_bb = list_entry(p_node, mir_basic_block, node);
        if (p_bb->if_visited) continue;

        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_bb->instr_list) {
            p_mir_instr p_instr = list_entry(p_instr_node, mir_instr, node);
            p_mir_vreg p_des = NULL;
            switch (p_instr->irkind) {
            case mir_add_op:
            case mir_sub_op:
            case mir_mul_op:
            case mir_div_op:
            case mir_mod_op:
            case mir_and_op:
            case mir_or_op:
            case mir_g_op:
            case mir_geq_op:
            case mir_l_op:
            case mir_leq_op:
            case mir_eq_op:
            case mir_neq_op:
                p_des = p_instr->mir_binary.p_des;
                break;
            case mir_minus_op:
            case mir_not_op:
            case mir_val_assign:
                p_des = p_instr->mir_unary.p_des;
                break;
            case mir_call:
                p_des = p_instr->mir_call.p_des;
                break;
            case mir_load:
                p_des = p_instr->mir_load.p_des;
            case mir_store:
                break;
            case mir_alloca:
                p_des = p_instr->mir_alloca.p_des;
                break;
            }
            if (p_des) {
                list_del(&p_des->node);
                mir_vreg_drop(p_des);
            }
        }

        p_mir_basic_block_branch_target p_target_1 = p_bb->p_branch->p_target_1;
        p_mir_basic_block_branch_target p_target_2 = p_bb->p_branch->p_target_2;
        p_list_head p_prev_node;
        if (p_target_1) {
            list_for_each(p_prev_node, &p_target_1->p_block->prev_basic_block_list) {
                p_mir_basic_block_list_node p_bbl = list_entry(p_prev_node, mir_basic_block_list_node, node);
                if (p_bbl->p_basic_block == p_bb) {
                    list_del(p_prev_node);
                    free(p_bbl);
                    break;
                }
            }
        }
        if (p_target_2) {
            list_for_each(p_prev_node, &p_target_2->p_block->prev_basic_block_list) {
                p_mir_basic_block_list_node p_bbl = list_entry(p_prev_node, mir_basic_block_list_node, node);
                if (p_bbl->p_basic_block == p_bb) {
                    list_del(p_prev_node);
                    free(p_bbl);
                    break;
                }
            }
        }
    }
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->entry_block) {
        p_mir_basic_block p_bb = list_entry(p_node, mir_basic_block, node);
        if (p_bb->if_visited) continue;

        list_del(&p_bb->node);
        mir_basic_block_drop(p_bb);
    }
}

static inline void mir_simplify_cfg_func_merge_single_predecessor_bb(p_mir_func p_func) {
    p_list_head p_node;
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->entry_block) {
        p_mir_basic_block p_bb = list_entry(p_node, mir_basic_block, node);
        if ((&p_bb->prev_basic_block_list)->p_next->p_next != &p_bb->prev_basic_block_list) continue;
        if (p_node == p_func->entry_block.p_next) continue;
        assert(!list_head_alone(&p_bb->prev_basic_block_list));

        p_mir_basic_block p_prev_bb = list_entry(p_bb->prev_basic_block_list.p_prev, mir_basic_block_list_node, node)->p_basic_block;
        if (p_prev_bb->p_branch->kind != mir_br_branch) continue;
        assert(p_prev_bb != p_bb);

        p_mir_basic_block_branch_target p_target_1 = p_bb->p_branch->p_target_1;
        p_mir_basic_block_branch_target p_target_2 = p_bb->p_branch->p_target_2;
        if (p_target_1) {
            p_list_head p_node;
            list_for_each(p_node, &p_target_1->p_block->prev_basic_block_list) {
                p_mir_basic_block_list_node p_bbl = list_entry(p_node, mir_basic_block_list_node, node);
                if (p_bbl->p_basic_block == p_bb) {
                    p_bbl->p_basic_block = p_prev_bb;
                    break;
                }
            }
        }
        if (p_target_2) {
            p_list_head p_node;
            list_for_each(p_node, &p_target_2->p_block->prev_basic_block_list) {
                p_mir_basic_block_list_node p_bbl = list_entry(p_node, mir_basic_block_list_node, node);
                if (p_bbl->p_basic_block == p_bb) {
                    p_bbl->p_basic_block = p_prev_bb;
                    break;
                }
            }
        }

        if (!list_blk_add_prev(&p_bb->instr_list, &p_prev_bb->instr_list))
            list_replace(&p_prev_bb->instr_list, &p_bb->instr_list);
        p_bb->instr_list.p_next = &p_bb->instr_list;
        p_bb->instr_list.p_prev = &p_bb->instr_list;

        p_mir_basic_block_branch p_tmp_branch = p_prev_bb->p_branch;
        p_prev_bb->p_branch = p_bb->p_branch;
        p_bb->p_branch = p_tmp_branch;

        list_del(&p_bb->node);
        mir_basic_block_drop(p_bb);
    }
}

static inline void mir_simplify_cfg_func_eliminate_single_br_bb(p_mir_func p_func) {
    // TODO solve ssa bb param
    p_list_head p_node;
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->entry_block) {
        p_mir_basic_block p_bb = list_entry(p_node, mir_basic_block, node);
        if (!list_head_alone(&p_bb->instr_list)) continue;

        p_mir_basic_block_branch_target p_target = p_bb->p_branch->p_target_1;
        if (!p_target || p_bb->p_branch->p_target_2) continue;
        if (p_target->p_block == p_bb) continue;

        p_list_head p_prev_node;
        list_for_each(p_prev_node, &p_target->p_block->prev_basic_block_list) {
            p_mir_basic_block_list_node p_bbl = list_entry(p_prev_node, mir_basic_block_list_node, node);
            if (p_bbl->p_basic_block == p_bb) {
                list_del(p_prev_node);
                free(p_bbl);
                break;
            }
        }

        list_for_each(p_prev_node, &p_bb->prev_basic_block_list) {
            p_mir_basic_block p_prev_bb = list_entry(p_prev_node, mir_basic_block_list_node, node)->p_basic_block;
            p_mir_basic_block_branch_target p_prev_target_1 = p_prev_bb->p_branch->p_target_1;
            p_mir_basic_block_branch_target p_prev_target_2 = p_prev_bb->p_branch->p_target_2;

            assert(p_prev_target_1);
            if (p_prev_target_2 && p_prev_target_2->p_block == p_bb) {
                p_mir_basic_block_branch_target p_tmp = p_prev_target_2;
                p_prev_target_2 = p_prev_target_1;
                p_prev_target_1 = p_tmp;
            }

            if (p_prev_target_2 && p_prev_target_2->p_block == p_target->p_block) {
                p_prev_bb->p_branch->kind = mir_br_branch;
                p_prev_bb->p_branch->p_target_1 = p_prev_target_2;
                mir_operand_drop(p_prev_bb->p_branch->p_exp);
                mir_basic_block_branch_target_drop(p_prev_target_1);
                continue;
            }
            mir_basic_block_add_prev(p_prev_bb, p_target->p_block);
            p_prev_target_1->p_block = p_target->p_block;
        }

        list_del(&p_bb->node);
        mir_basic_block_drop(p_bb);
    }
}

static inline void mir_simplify_cfg_func_pass(p_mir_func p_func) {
    if (list_head_alone(&p_func->entry_block)) return;

    mir_simplify_cfg_func_remove_no_predesessor_bb(p_func);
    // TODO mir_simplify_func_eliminate_single_predecessor_phi(p_func);
    mir_simplify_cfg_func_merge_single_predecessor_bb(p_func);
    mir_simplify_cfg_func_eliminate_single_br_bb(p_func);
    return;
}

void mir_simplify_cfg_pass(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_mir_func p_func = list_entry(p_node, symbol_sym, node)->p_m_func;
        mir_simplify_cfg_func_pass(p_func);

        mir_func_set_block_id(p_func);
        mir_func_set_vreg_id(p_func);
    }
}
