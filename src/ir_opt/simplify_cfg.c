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

static inline void ir_simplify_cfg_func_remove_no_predesessor_bb(p_symbol_func p_func) {
    symbol_func_basic_block_init_visited(p_func);
    p_ir_basic_block p_entry_bb = list_entry(p_func->block.p_next, ir_basic_block, node);
    ir_simplify_cfg_dfs_basic_block(p_entry_bb);
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if (p_bb->if_visited) continue;

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
                symbol_func_vreg_del(p_func, p_des);
            }
        }

        p_ir_basic_block_branch_target p_target_1 = p_bb->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_target_2 = p_bb->p_branch->p_target_2;
        p_list_head p_prev_node;
        if (p_target_1) {
            list_for_each(p_prev_node, &p_target_1->p_block->prev_basic_block_list) {
                p_ir_basic_block_list_node p_bbl = list_entry(p_prev_node, ir_basic_block_list_node, node);
                if (p_bbl->p_basic_block == p_bb) {
                    list_del(p_prev_node);
                    free(p_bbl);
                    break;
                }
            }
        }
        if (p_target_2) {
            list_for_each(p_prev_node, &p_target_2->p_block->prev_basic_block_list) {
                p_ir_basic_block_list_node p_bbl = list_entry(p_prev_node, ir_basic_block_list_node, node);
                if (p_bbl->p_basic_block == p_bb) {
                    list_del(p_prev_node);
                    free(p_bbl);
                    break;
                }
            }
        }
    }
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if (p_bb->if_visited) continue;

        symbol_func_bb_del(p_func, p_bb);
    }
}

static inline void ir_simplify_cfg_func_merge_single_predecessor_bb(p_symbol_func p_func) {
    p_list_head p_node;
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if ((&p_bb->prev_basic_block_list)->p_next->p_next != &p_bb->prev_basic_block_list) continue;
        if (p_node == p_func->block.p_next) continue;
        assert(!list_head_alone(&p_bb->prev_basic_block_list));

        p_ir_basic_block p_prev_bb = list_entry(p_bb->prev_basic_block_list.p_prev, ir_basic_block_list_node, node)->p_basic_block;
        if (p_prev_bb->p_branch->kind != ir_br_branch) continue;
        assert(p_prev_bb != p_bb);

        p_ir_basic_block_branch_target p_target_1 = p_bb->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_target_2 = p_bb->p_branch->p_target_2;
        if (p_target_1) {
            p_list_head p_node;
            list_for_each(p_node, &p_target_1->p_block->prev_basic_block_list) {
                p_ir_basic_block_list_node p_bbl = list_entry(p_node, ir_basic_block_list_node, node);
                if (p_bbl->p_basic_block == p_bb) {
                    p_bbl->p_basic_block = p_prev_bb;
                    break;
                }
            }
        }
        if (p_target_2) {
            p_list_head p_node;
            list_for_each(p_node, &p_target_2->p_block->prev_basic_block_list) {
                p_ir_basic_block_list_node p_bbl = list_entry(p_node, ir_basic_block_list_node, node);
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

        p_ir_basic_block_branch p_tmp_branch = p_prev_bb->p_branch;
        p_prev_bb->p_branch = p_bb->p_branch;
        p_bb->p_branch = p_tmp_branch;

        symbol_func_bb_del(p_func, p_bb);
    }
}

static inline bool ir_simplify_cfg_func_eliminate_single_br_bb(p_symbol_func p_func) {
    bool if_del = false;

    // TODO solve ssa bb param
    p_list_head p_node;
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if (!list_head_alone(&p_bb->instr_list)) continue;

        p_ir_basic_block_branch_target p_target = p_bb->p_branch->p_target_1;
        if (!p_target || p_bb->p_branch->p_target_2) continue;
        if (p_target->p_block == p_bb) continue;

        p_list_head p_prev_node;
        list_for_each(p_prev_node, &p_target->p_block->prev_basic_block_list) {
            p_ir_basic_block_list_node p_bbl = list_entry(p_prev_node, ir_basic_block_list_node, node);
            if (p_bbl->p_basic_block == p_bb) {
                list_del(p_prev_node);
                free(p_bbl);
                break;
            }
        }

        list_for_each(p_prev_node, &p_bb->prev_basic_block_list) {
            p_ir_basic_block p_prev_bb = list_entry(p_prev_node, ir_basic_block_list_node, node)->p_basic_block;
            p_ir_basic_block_branch_target p_prev_target_1 = p_prev_bb->p_branch->p_target_1;
            p_ir_basic_block_branch_target p_prev_target_2 = p_prev_bb->p_branch->p_target_2;

            assert(p_prev_target_1);
            if (p_prev_target_2) {
                if_del = true;

                if (p_prev_target_2->p_block == p_bb) {
                    p_ir_basic_block_branch_target p_tmp = p_prev_target_2;
                    p_prev_target_2 = p_prev_target_1;
                    p_prev_target_1 = p_tmp;
                }

                if (p_prev_target_2->p_block == p_target->p_block) {
                    p_prev_bb->p_branch->kind = ir_br_branch;
                    p_prev_bb->p_branch->p_target_1 = p_prev_target_2;
                    ir_operand_drop(p_prev_bb->p_branch->p_exp);
                    p_prev_bb->p_branch->p_exp = NULL;
                    ir_basic_block_branch_target_drop(p_prev_target_1);
                    p_prev_bb->p_branch->p_target_2 = NULL;
                    continue;
                }
            }
            ir_basic_block_add_prev(p_prev_bb, p_target->p_block);
            p_prev_target_1->p_block = p_target->p_block;
        }

        symbol_func_bb_del(p_func, p_bb);
    }
    return if_del;
}

static inline bool ir_simplify_cfg_func_pass(p_symbol_func p_func) {
    if (list_head_alone(&p_func->block)) return false;

    bool if_del;
    ir_simplify_cfg_func_remove_no_predesessor_bb(p_func);
    // TODO ir_simplify_func_eliminate_single_predecessor_phi(p_func);
    ir_simplify_cfg_func_merge_single_predecessor_bb(p_func);
    if_del = ir_simplify_cfg_func_eliminate_single_br_bb(p_func);
    return if_del;
}

bool ir_simplify_cfg_pass(p_program p_program) {
    bool if_del = false;
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if_del |= ir_simplify_cfg_func_pass(p_func);

        symbol_func_set_block_id(p_func);
    }
    return if_del;
}
