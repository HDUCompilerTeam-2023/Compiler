#include <program/use.h>
#include <program/def.h>
#include <symbol_gen/func.h>
#include <symbol_gen/type.h>
#include <symbol_gen/var.h>
#include <ir_manager/builddomtree.h>
#include <ir_manager/liveness.h>
#include <ir_opt/cssa.h>
#include <ir_opt/deadcode_elimate.h>
#include <ir_gen/basic_block.h>
#include <ir_gen/operand.h>
#include <ir_gen/instr.h>
#include <ir_gen/vreg.h>
#include <ir/bb_param.h>
#include <ir/param.h>

#include <ir_print/instr.h>

#define R_REG_PARAM_CNT 4
#define S_REG_PARAM_CNT 16
#define R_REG_NUM 13
#define S_REG_NUM 32

typedef struct {
    p_ir_basic_block p_bb;
    list_head node;
} bb_rpo_node, *p_bb_rpo_node;
static inline p_bb_rpo_node _bb_rpo_node_gen(p_ir_basic_block p_bb) {
    p_bb_rpo_node p_rn = malloc(sizeof(*p_rn));
    *p_rn = (bb_rpo_node) {
        .p_bb = p_bb,
        .node = list_init_head(&p_rn->node),
    };
    return p_rn;
}
static inline p_ir_basic_block _bb_rpo_node_pop(p_list_head p_head) {
    if (list_head_alone(p_head))
        return NULL;
    p_bb_rpo_node p_rn = list_entry(p_head->p_next, bb_rpo_node, node);
    list_del(&p_rn->node);
    p_ir_basic_block p_bb = p_rn->p_bb;
    free(p_rn);
    return p_bb;
}
static inline p_ir_basic_block _bb_rpo_node_pop_tail(p_list_head p_head) {
    if (list_head_alone(p_head))
        return NULL;
    p_bb_rpo_node p_rn = list_entry(p_head->p_prev, bb_rpo_node, node);
    list_del(&p_rn->node);
    p_ir_basic_block p_bb = p_rn->p_bb;
    free(p_rn);
    return p_bb;
}
static inline void _rpo_walk_bb(p_ir_basic_block p_bb, p_list_head p_head) {
    if (p_bb->if_visited)
        return;
    p_bb->if_visited = true;

    if (p_bb->p_branch->p_target_1)
        _rpo_walk_bb(p_bb->p_branch->p_target_1->p_block, p_head);
    if (p_bb->p_branch->p_target_2)
        _rpo_walk_bb(p_bb->p_branch->p_target_2->p_block, p_head);

    p_bb_rpo_node p_rn = _bb_rpo_node_gen(p_bb);
    list_add_next(&p_rn->node, p_head);
}
static inline p_list_head _rpo_bb_list_gen(p_symbol_func p_func) {
    p_list_head bb_head = malloc(sizeof(list_head));
    *bb_head = list_init_head(bb_head);

    symbol_func_basic_block_init_visited(p_func);
    _rpo_walk_bb(p_func->p_entry_block, bb_head);

    return bb_head;
}
static inline void _rpo_bb_list_drop(p_list_head bb_head) {
    assert(list_head_alone(bb_head));
    free(bb_head);
}

p_bitmap *_compute_dom_frontier(p_symbol_func p_func) {
    ir_cfg_set_func_dom(p_func);

    p_bitmap *dom_frontier_map = malloc(sizeof(p_bitmap) * p_func->block_cnt);
    for (size_t i = 0; i < p_func->block_cnt; ++i) {
        dom_frontier_map[i] = bitmap_gen(p_func->block_cnt);
        bitmap_set_empty(dom_frontier_map[i]);
    }

    p_list_head bb_head = _rpo_bb_list_gen(p_func);

    p_ir_basic_block p_basic_block;
    while(p_basic_block = _bb_rpo_node_pop_tail(bb_head), p_basic_block) {
        printf("deal b%ld\n", p_basic_block->block_id);
        p_bitmap dom_frontier = dom_frontier_map[p_basic_block->block_id];
        p_ir_basic_block_branch_target p_true_block = p_basic_block->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_false_block = p_basic_block->p_branch->p_target_2;

        // 将直接后继做为 DF_up 的候选
        if (p_true_block)
            bitmap_add_element(dom_frontier, p_true_block->p_block->block_id);
        if (p_false_block)
            bitmap_add_element(dom_frontier, p_false_block->p_block->block_id);

        p_list_head p_node;
        // 记录 直接支配点
        p_bitmap p_son_list = bitmap_gen(p_func->block_cnt);
        bitmap_set_empty(p_son_list);
        // 将支配树上的直接儿子的支配边界作为候选
        list_for_each(p_node, &p_basic_block->dom_son_list->block_list) {
            p_ir_basic_block p_son = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            bitmap_add_element(p_son_list, p_son->block_id);
            bitmap_merge_not_new(dom_frontier, dom_frontier_map[p_son->block_id]);
        }
        printf("son list for b%ld\n", p_basic_block->block_id);
        for (size_t i = 0; i < p_func->block_cnt; ++i) {
            if (bitmap_if_in(p_son_list, i)) {
                printf("b%ld ", i);
            }
        }
        printf("candidate dom frontier for b%ld\n", p_basic_block->block_id);
        for (size_t i = 0; i < p_func->block_cnt; ++i) {
            if (bitmap_if_in(dom_frontier, i)) {
                printf("b%ld ", i);
            }
        }
        // 所有候选中不受当前节点直接支配的节点为支配边界
        bitmap_neg_not_new(p_son_list);
        bitmap_and_not_new(dom_frontier, p_son_list);
        bitmap_drop(p_son_list);

        printf("dom frontier for b%ld\n", p_basic_block->block_id);
        for (size_t i = 0; i < p_func->block_cnt; ++i) {
            if (bitmap_if_in(dom_frontier, i)) {
                printf("b%ld ", i);
            }
        }
        printf("\n");
    }
    _rpo_bb_list_drop(bb_head);

    return dom_frontier_map;
}

typedef struct {
    p_symbol_var p_var;
    p_phi_congruence_class pcc;
    list_head node;
} spill_slot_node, *p_spill_slot_node;
static inline p_spill_slot_node _spill_slot_node_gen (p_phi_congruence_class pcc) {
    p_spill_slot_node p_ssn = malloc(sizeof(*p_ssn));
    p_symbol_var p_var = symbol_temp_var_gen(symbol_type_copy(pcc->p_mem_type));

    *p_ssn = (spill_slot_node) {
        .pcc = pcc,
        .p_var = p_var,
        .node = list_init_head(&p_ssn->node),
    };

    p_list_head p_node;
    list_for_each(p_node, &pcc->set) {
        p_pcc_node p_pn = list_entry(p_node, pcc_node, node);
        p_pn->p_vreg->p_info = p_ssn->p_var;
    }

    return p_ssn;
}
static inline void _spill_slot_node_drop(p_spill_slot_node p_ssn) {
    list_del(&p_ssn->node);
    if (list_head_alone(&p_ssn->p_var->ref_list)) {
        p_list_head p_node;
        list_for_each(p_node, &p_ssn->pcc->set) {
            p_pcc_node p_pn = list_entry(p_node, pcc_node, node);
            p_pn->p_vreg->p_info = NULL;
        }
    }
    phi_congruence_class_drop(p_ssn->pcc);
    free(p_ssn);
}

typedef struct {
    p_symbol_func p_func;
    list_head list;
    size_t cnt;
} spill_slot, *p_spill_slot;
static inline p_spill_slot _spill_slot_gen(p_symbol_func p_func) {
    p_spill_slot p_slot = malloc(sizeof(*p_slot));
    *p_slot = (spill_slot) {
        .p_func = p_func,
        .list = list_init_head(&p_slot->list),
        .cnt = 0,
    };
    return p_slot;
}
static inline void _spill_slot_drop(p_spill_slot p_slot) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_slot->list) {
        p_spill_slot_node p_ssn = list_entry(p_node, spill_slot_node, node);
        _spill_slot_node_drop(p_ssn);
    }
    list_for_each_safe(p_node, p_next, &p_slot->p_func->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        if (list_head_alone(&p_var->ref_list)) {
            symbol_var_drop(p_var);
        }
    }
    free(p_slot);
}

static inline p_spill_slot _spill_slot_alloc(p_symbol_func p_func) {
    list_head pcc_head = list_init_head(&pcc_head);
    ir_opt_cssa(p_func, &pcc_head);

    p_spill_slot p_slot = _spill_slot_gen(p_func);

    size_t id = 0;
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &pcc_head) {
        p_phi_congruence_class pcc = list_entry(p_node, phi_congruence_class, node);
        p_spill_slot_node p_ssn = _spill_slot_node_gen(pcc);
        symbol_func_add_variable(p_func, p_ssn->p_var);
        list_add_prev(&p_ssn->node, &p_slot->list);
        p_ssn->p_var->id = id++;
    }
    p_slot->cnt = id;

    size_t r = 0;
    size_t s = 0;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_param_vreg = list_entry(p_node, ir_vreg, node);
        p_symbol_var p_vmem = p_param_vreg->p_info;
        if (p_param_vreg->p_type->ref_level || p_param_vreg->p_type->basic != type_f32) {
            if (++r > R_REG_PARAM_CNT) {
                list_del(&p_vmem->node);
                list_add_prev(&p_vmem->node, &p_func->param);
            }
        }
        else if (++s > S_REG_PARAM_CNT) {
            list_del(&p_vmem->node);
            list_add_prev(&p_vmem->node, &p_func->param);
        }
    }

    return p_slot;
}

typedef struct {
    size_t block_id;
    list_head node;
} wl_node, *p_wl_node;
static inline void _wl_add(p_list_head p_head, size_t block_id) {
    p_list_head p_node;
    list_for_each(p_node, p_head) {
        p_wl_node p_wn = list_entry(p_node, wl_node, node);
        if (p_wn->block_id == block_id)
            return;
    }
    p_wl_node p_wn = malloc(sizeof(*p_wn));
    *p_wn = (wl_node) {
        .block_id = block_id,
        .node = list_init_head(&p_wn->node),
    };
    list_add_prev(&p_wn->node, p_head);
}
static inline size_t _wl_pop(p_list_head p_head) {
    if (list_head_alone(p_head))
        return -1;
    p_wl_node p_wn = list_entry(p_head->p_next, wl_node, node);
    list_del(&p_wn->node);
    size_t ret = p_wn->block_id;
    free(p_wn);
    return ret;
}

static inline bool _book(size_t id, bool *set) {
    if (set[id])
        return false;
    set[id] = true;
    return true;
}
static inline void _retain_ssa(p_spill_slot p_slot) {
    p_symbol_func p_func = p_slot->p_func;
    size_t pcc_cnt = p_slot->cnt;

    list_head wl_head = list_init_head(&wl_head);

    bool *has_def = malloc(sizeof(bool) * p_func->block_cnt * pcc_cnt);
    memset(has_def, 0, sizeof(bool) * p_func->block_cnt * pcc_cnt);
    bool *need_phi = malloc(sizeof(bool) * p_func->block_cnt * pcc_cnt);
    memset(need_phi, 0, sizeof(bool) * p_func->block_cnt * pcc_cnt);

    p_list_head p_node;
    size_t r = 0;
    size_t s = 0;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->p_type->ref_level || p_vreg->p_type->basic != type_f32) {
            if (++r > R_REG_PARAM_CNT) continue;
        }
        else if (++s > S_REG_PARAM_CNT) continue;
        p_symbol_var p_var = p_vreg->p_info;
        _book(p_var->id, has_def + p_func->p_entry_block->block_id * pcc_cnt);
        _wl_add(&wl_head, p_func->p_entry_block->block_id);
    }
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        bool *has_def_bb = has_def + p_bb->block_id * pcc_cnt;

        p_list_head p_node;
        list_for_each(p_node, &p_bb->basic_block_phis) {
            p_ir_vreg p_vreg = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
            p_symbol_var p_var = p_vreg->p_info;
            _book(p_var->id, has_def_bb);
            _wl_add(&wl_head, p_bb->block_id);
        }
        list_for_each(p_node, &p_bb->instr_list) {
            p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
            p_ir_vreg p_des = NULL;
            switch (p_instr->irkind) {
            case ir_binary:
                p_des = p_instr->ir_binary.p_des;
                break;
            case ir_unary:
                p_des = p_instr->ir_unary.p_des;
                break;
            case ir_load:
                p_des = p_instr->ir_load.p_des;
                break;
            case ir_store:
                break;
            case ir_call:
                p_des = p_instr->ir_call.p_des;
                break;
            case ir_gep:
                assert(0);
                break;
            }
            if (p_des) {
                p_symbol_var p_var = p_des->p_info;
                _book(p_var->id, has_def_bb);
            }
            _wl_add(&wl_head, p_bb->block_id);
        }
    }

    p_bitmap *dom_frontier_map = _compute_dom_frontier(p_func);

    size_t deal_bb_id;
    while(deal_bb_id = _wl_pop(&wl_head), deal_bb_id != -1) {
        bool *has_def_deal_bb = has_def + deal_bb_id * pcc_cnt;
        printf("deal b%ld\n", deal_bb_id);
        for(size_t df_bb_id = 0; df_bb_id < p_func->block_cnt; ++df_bb_id) {
            if (bitmap_if_in(dom_frontier_map[deal_bb_id], df_bb_id)) {
                printf("  dom frontier b%ld\n", df_bb_id);
                bool *has_def_df_bb = has_def + df_bb_id * pcc_cnt;
                bool *need_phi_df_bb = need_phi + df_bb_id * pcc_cnt;
                for (size_t i = 0; i < pcc_cnt; ++i) {
                    if (!has_def_deal_bb[i])
                        continue;
                    if (!_book(i, need_phi_df_bb))
                        continue;
                    _book(i, has_def_df_bb);
                    printf("    add %%%ld\n", i);
                    _wl_add(&wl_head, df_bb_id);
                }
            }
        }
    }

    list_for_each(p_node, &p_slot->list) {
        p_spill_slot_node p_ssn = list_entry(p_node, spill_slot_node, node);
        p_symbol_var p_var = p_ssn->p_var;
        p_phi_congruence_class pcc = p_ssn->pcc;
        bool have_print = false;

        p_list_head p_node;
        list_for_each(p_node, &pcc->set) {
            p_ir_vreg p_vreg = list_entry(p_node, pcc_node, node)->p_vreg;
            assert(p_vreg->p_info == p_var);

            p_list_head p_use_node, p_use_next;
            list_for_each_safe(p_use_node, p_use_next, &p_vreg->use_list) {
                p_ir_operand p_use = list_entry(p_use_node, ir_operand, use_node);
                p_ir_basic_block p_use_bb = NULL;
                p_list_head p_instr_node = NULL;
                switch (p_use->used_type) {
                case instr_ptr:
                    p_use_bb = p_use->p_instr->p_basic_block;
                    p_instr_node = &p_use->p_instr->node;
                    break;
                case bb_param_ptr:
                    p_use_bb = p_use->p_bb_param->p_target->p_source_block;
                    p_instr_node = &p_use_bb->instr_list;
                    break;
                case cond_ptr:
                case ret_ptr:
                    p_use_bb = p_use->p_basic_block;
                    p_instr_node = &p_use_bb->instr_list;
                    break;
                }
                assert(p_use_bb);

                p_ir_basic_block p_dom_def_bb = p_use_bb;
                while (1) {
                    bool *has_def_bb = has_def + p_dom_def_bb->block_id * pcc_cnt;
                    bool *need_phi_bb = need_phi + p_dom_def_bb->block_id * pcc_cnt;
                    if (!has_def_bb[p_var->id]) {
                        assert(!need_phi_bb[p_var->id]);
                        p_dom_def_bb = p_dom_def_bb->p_dom_parent;
                        p_instr_node = &p_dom_def_bb->instr_list;
                        continue;
                    }
                    bool finded = false;
                    p_list_head p_node;
                    list_for_each_tail(p_node, p_instr_node) {
                        if (p_node == &p_dom_def_bb->instr_list) {
                            break;
                        }
                        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
                        p_ir_vreg p_des = NULL;
                        switch (p_instr->irkind) {
                        case ir_binary:
                            p_des = p_instr->ir_binary.p_des;
                            break;
                        case ir_unary:
                            p_des = p_instr->ir_unary.p_des;
                            break;
                        case ir_load:
                            p_des = p_instr->ir_load.p_des;
                            break;
                        case ir_store:
                            break;
                        case ir_call:
                            p_des = p_instr->ir_call.p_des;
                            break;
                        case ir_gep:
                            assert(0);
                            break;
                        }
                        if (p_des && p_des->p_info == p_var) {
                            finded = true;
                            if (p_des != p_vreg) {
                                if (!have_print) {
                                    have_print = true;
                                    printf("deal pcc spill to var %ld\n", p_var->id);
                                    phi_congruence_class_print(pcc);
                                }
                                ir_operand_reset_vreg(p_use, p_des);
                                printf("for %%%ld use in b%ld find instr def in b%ld\n", p_vreg->id, p_use_bb->block_id, p_dom_def_bb->block_id);
                            }
                            break;
                        }
                    }
                    if (finded)
                        break;
                    list_for_each(p_node, &p_dom_def_bb->basic_block_phis) {
                        p_ir_vreg p_des = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
                        if (p_des->p_info == p_var) {
                            finded = true;
                            if (p_des != p_vreg) {
                                if (!have_print) {
                                    have_print = true;
                                    printf("deal pcc spill to var %ld\n", p_var->id);
                                    phi_congruence_class_print(pcc);
                                }
                                ir_operand_reset_vreg(p_use, p_des);
                                printf("for %%%ld use in b%ld find phi def in b%ld\n", p_vreg->id, p_use_bb->block_id, p_dom_def_bb->block_id);
                            }
                            break;
                        }
                    }
                    if (finded)
                        break;
                    if (p_dom_def_bb == p_func->p_entry_block) {
                        size_t r = 0;
                        size_t s = 0;
                        list_for_each(p_node, &p_func->param_reg_list) {
                            p_ir_vreg p_des = list_entry(p_node, ir_vreg, node);
                            if (p_des->p_type->ref_level || p_des->p_type->basic != type_f32) {
                                if (++r > R_REG_PARAM_CNT) continue;
                            }
                            else if (++s > S_REG_PARAM_CNT) continue;
                            if (p_des->p_info == p_var) {
                                finded = true;
                                if (p_des != p_vreg) {
                                    if (!have_print) {
                                        have_print = true;
                                        printf("deal pcc spill to var %ld\n", p_var->id);
                                        phi_congruence_class_print(pcc);
                                    }
                                    ir_operand_reset_vreg(p_use, p_des);
                                    printf("for %%%ld use in b%ld find func param des\n", p_vreg->id, p_use_bb->block_id);
                                }
                                break;
                            }
                        }
                    }
                    if (finded)
                        break;
                    if (need_phi_bb[p_var->id]) {
                        if (!have_print) {
                            have_print = true;
                            printf("deal pcc spill to var %ld\n", p_var->id);
                            phi_congruence_class_print(pcc);
                        }
                        printf("for %%%ld use in b%ld add phi def in b%ld\n", p_vreg->id, p_use_bb->block_id, p_dom_def_bb->block_id);
                        p_ir_vreg p_phi = ir_vreg_copy(p_vreg);
                        symbol_func_vreg_add(p_func, p_phi);
                        ir_basic_block_add_phi(p_dom_def_bb, p_phi);
                        p_list_head p_node;
                        list_for_each(p_node, &p_dom_def_bb->prev_branch_target_list) {
                            p_ir_basic_block_branch_target p_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
                            ir_basic_block_branch_target_add_param(p_target, ir_operand_vreg_gen(p_vreg));
                        }
                        p_use_next = p_use_node->p_next;
                        ir_operand_reset_vreg(p_use, p_phi);
                        need_phi_bb[p_var->id] = false;
                        break;
                    }
                    p_dom_def_bb = p_dom_def_bb->p_dom_parent;
                    p_instr_node = &p_dom_def_bb->instr_list;
                }
            }
        }
    }

    r = 0;
    s = 0;
    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_func->param_reg_list) {
        p_ir_vreg p_param_vreg = list_entry(p_node, ir_vreg, node);
        if (p_param_vreg->p_type->ref_level || p_param_vreg->p_type->basic != type_f32) {
            if (++r > R_REG_PARAM_CNT) {
                symbol_func_param_reg_del(p_func, p_param_vreg);
            }
        }
        else if (++s > S_REG_PARAM_CNT) {
            symbol_func_param_reg_del(p_func, p_param_vreg);
        }
    }

    for (size_t i = 0; i < p_func->block_cnt; ++i) {
        bitmap_drop(dom_frontier_map[i]);
    }
    free(dom_frontier_map);
    free(has_def);
    free(need_phi);
}

typedef struct {
    p_ir_vreg p_vreg;
    list_head node;
} vreg_set_node, *p_vreg_set_node;;
static inline p_vreg_set_node _vreg_set_node_gen(p_ir_vreg p_vreg) {
    p_vreg_set_node p_vsn = malloc(sizeof(*p_vsn));
    *p_vsn = (vreg_set_node) {
        .p_vreg = p_vreg,
        .node = list_init_head(&p_vsn->node),
    };
    return p_vsn;
}
static inline void _vreg_set_node_drop(p_vreg_set_node p_vsn) {
    list_del(&p_vsn->node);
    free(p_vsn);
}
typedef struct {
    list_head vregs;
    size_t cnt;
} vreg_set, *p_vreg_set;
static inline p_vreg_set _vreg_set_gen(void) {
    p_vreg_set p_vs = malloc(sizeof(*p_vs));
    *p_vs = (vreg_set) {
        .vregs = list_init_head(&p_vs->vregs),
        .cnt = 0,
    };
    return p_vs;
}
static inline bool _vreg_set_add(p_vreg_set p_vs, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_vs->vregs) {
        p_vreg_set_node p_vsn = list_entry(p_node, vreg_set_node, node);
        if (p_vsn->p_vreg == p_vreg) {
            return false;
        }
    }
    p_vreg_set_node p_vsn = _vreg_set_node_gen(p_vreg);
    list_add_prev(&p_vsn->node, &p_vs->vregs);
    ++p_vs->cnt;
    return true;
}
static inline bool _vreg_set_del(p_vreg_set p_vs, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_vs->vregs) {
        p_vreg_set_node p_vsn = list_entry(p_node, vreg_set_node, node);
        if (p_vsn->p_vreg == p_vreg) {
            _vreg_set_node_drop(p_vsn);
            --p_vs->cnt;
            return true;
        }
    }
    return false;
}
static inline void _vreg_set_del_tail(p_vreg_set p_vs) {
    assert(p_vs->cnt);
    assert(!list_head_alone(&p_vs->vregs));
    p_vreg_set_node p_vsn = list_entry(p_vs->vregs.p_prev, vreg_set_node, node);
    _vreg_set_node_drop(p_vsn);
    --p_vs->cnt;
}
static inline void _vreg_set_cut(p_vreg_set p_vs, size_t cnt) {
    while(p_vs->cnt > cnt)
        _vreg_set_del_tail(p_vs);
}
static inline void _vreg_set_drop(p_vreg_set p_vs) {
    while(p_vs->cnt)
        _vreg_set_del_tail(p_vs);
    free(p_vs);
}
static inline void _vreg_set_insertion(p_vreg_set p_des, p_vreg_set p_vs) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_des->vregs) {
        p_vreg_set_node p_vsn_des = list_entry(p_node, vreg_set_node, node);
        p_list_head p_node;
        list_for_each(p_node, &p_vs->vregs) {
            p_vreg_set_node p_vsn = list_entry(p_node, vreg_set_node, node);
            if (p_vsn->p_vreg == p_vsn_des->p_vreg)
                break;
        }
        if (p_node == &p_vs->vregs) {
            _vreg_set_node_drop(p_vsn_des);
            --p_des->cnt;
        }
    }
}
static inline bool _vreg_set_find(p_vreg_set p_vs, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_vs->vregs) {
        p_vreg_set_node p_vsn = list_entry(p_node, vreg_set_node, node);
        if (p_vsn->p_vreg == p_vreg)
            return true;
    }
    return false;
}
static inline void _vreg_set_union(p_vreg_set p_des, p_vreg_set p_vs) {
    p_list_head p_node;
    list_for_each(p_node, &p_vs->vregs) {
        p_vreg_set_node p_vsn = list_entry(p_node, vreg_set_node, node);
        _vreg_set_add(p_des, p_vsn->p_vreg);
    }
}
static inline void _vreg_set_complement(p_vreg_set p_des, p_vreg_set p_vs) {
    p_list_head p_node;
    list_for_each(p_node, &p_vs->vregs) {
        p_vreg_set_node p_vsn = list_entry(p_node, vreg_set_node, node);
        _vreg_set_del(p_des, p_vsn->p_vreg);
    }
}
static inline void _vreg_set_print(p_vreg_set p_vs) {
    printf("(%ld) {", p_vs->cnt);
    p_list_head p_node;
    list_for_each(p_node, &p_vs->vregs) {
        p_vreg_set_node p_vsn = list_entry(p_node, vreg_set_node, node);
        printf("%%%ld, ", p_vsn->p_vreg->id);
    }
    printf("}\n");
}

typedef struct {
    p_vreg_set vreg_set_r, vreg_set_s;
    size_t limit_r, limit_s;
} W, *p_W;
static inline p_W _W_gen(void) {
    p_W p_w = malloc(sizeof(*p_w));
    *p_w = (W) {
        .vreg_set_r = _vreg_set_gen(),
        .vreg_set_s = _vreg_set_gen(),
        .limit_r = R_REG_NUM,
        .limit_s = S_REG_NUM,
    };
    return p_w;
}
static inline void _W_drop(p_W p_w) {
    _vreg_set_drop(p_w->vreg_set_r);
    _vreg_set_drop(p_w->vreg_set_s);
    free(p_w);
}
static inline bool _W_add(p_W p_w, p_ir_vreg p_vreg) {
    if (p_vreg->if_float)
        return _vreg_set_add(p_w->vreg_set_s, p_vreg);
    else
        return _vreg_set_add(p_w->vreg_set_r, p_vreg);
}
static inline bool _W_del(p_W p_w, p_ir_vreg p_vreg) {
    if (p_vreg->if_float)
        return _vreg_set_del(p_w->vreg_set_s, p_vreg);
    else
        return _vreg_set_del(p_w->vreg_set_r, p_vreg);
}
static inline void _W_cut(p_W p_w, size_t space_r, size_t space_s) {
    if (p_w->limit_r < space_r)
        space_r = p_w->limit_r;
    if (p_w->limit_s < space_s)
        space_s = p_w->limit_s;
    _vreg_set_cut(p_w->vreg_set_r, p_w->limit_r - space_r);
    _vreg_set_cut(p_w->vreg_set_s, p_w->limit_s - space_s);
}
static inline bool _W_find(p_W p_w, p_ir_vreg p_vreg) {
    if (p_vreg->if_float)
        return _vreg_set_find(p_w->vreg_set_s, p_vreg);
    else
        return _vreg_set_find(p_w->vreg_set_s, p_vreg);
}
static inline void _W_insertion(p_W p_des, p_W p_w) {
    _vreg_set_insertion(p_des->vreg_set_r, p_w->vreg_set_r);
    _vreg_set_insertion(p_des->vreg_set_s, p_w->vreg_set_s);
}
static inline void _W_union(p_W p_des, p_W p_w) {
    _vreg_set_union(p_des->vreg_set_r, p_w->vreg_set_r);
    _vreg_set_union(p_des->vreg_set_s, p_w->vreg_set_s);
}
static inline void _W_complement(p_W p_des, p_W p_w) {
    _vreg_set_complement(p_des->vreg_set_r, p_w->vreg_set_r);
    _vreg_set_complement(p_des->vreg_set_s, p_w->vreg_set_s);
}
static inline void _W_print(p_W p_w) {
    _vreg_set_print(p_w->vreg_set_r);
    _vreg_set_print(p_w->vreg_set_s);
}
static inline bool _W_replace(p_W p_w, p_ir_vreg p_from, p_ir_vreg p_to) {
    if (_W_del(p_w, p_from)) {
        _W_add(p_w, p_to);
        return true;
    }
    return false;
}
static inline p_W _W_copy(p_W p_w) {
    p_W p_ret = _W_gen();
    _W_union(p_ret, p_w);
    return p_ret;
}

static inline p_ir_vreg_list _get_bb_live_in(p_ir_basic_block p_bb) {
    if (list_head_alone(&p_bb->instr_list))
        return p_bb->p_branch->p_live_in;
    p_ir_instr p_instr = list_entry(p_bb->instr_list.p_next, ir_instr, node);
    return p_instr->p_live_in;
}

static inline void _vreg_set_sort_by_next_use(p_vreg_set p_vs, p_ir_vreg_list p_live) {
    size_t cnt = p_vs->cnt;

    size_t *next_use_map = malloc(sizeof(*next_use_map) * cnt);
    p_vreg_set_node *vreg_node_map = malloc(sizeof(p_vreg_set_node) * cnt);

    size_t id = 0;
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_vs->vregs) {
        p_vreg_set_node p_vsn = list_entry(p_node, vreg_set_node, node);
        size_t next_use = -1;
        p_list_head p_node;
        list_for_each(p_node, &p_live->vreg_list) {
            p_ir_vreg_list_node p_ln = list_entry(p_node, ir_vreg_list_node, node);
            if (p_vsn->p_vreg == p_ln->p_vreg)
                next_use = p_ln->next_use;
        }
        list_del(&p_vsn->node);
        next_use_map[id] = next_use;
        vreg_node_map[id] = p_vsn;
        ++id;
    }
    assert(id == cnt);

    for (size_t i = 0; i < cnt; ++i) {
        size_t t = i;
        for(size_t j = i + 1; j < cnt; ++j) {
            if (next_use_map[j] < next_use_map[t])
                t = j;
        }
        if (t == i)
            continue;

        p_vreg_set_node p_tmp_vsn = vreg_node_map[t];
        vreg_node_map[t] = vreg_node_map[i];
        vreg_node_map[i] = p_tmp_vsn;

        size_t tmp_next_use = next_use_map[t];
        next_use_map[t] = next_use_map[i];
        next_use_map[i] = tmp_next_use;
    }

    for (size_t i = 0; i < cnt; ++i) {
        if (next_use_map[i] == -1) {
            _vreg_set_node_drop(vreg_node_map[i]);
            --p_vs->cnt;
            continue;
        }
        list_add_prev(&vreg_node_map[i]->node, &p_vs->vregs);
    }

    free(next_use_map);
    free(vreg_node_map);
}
static inline void _W_sort_by_next_use(p_W p_w, p_ir_vreg_list p_live) {
    _vreg_set_sort_by_next_use(p_w->vreg_set_r, p_live);
    _vreg_set_sort_by_next_use(p_w->vreg_set_s, p_live);
}

static inline p_W _W_init_usual(p_W *W_exit_map, p_ir_basic_block p_bb) {
    printf("init W entry b%ld for usual bb\n", p_bb->block_id);

    p_W take = _W_gen();
    p_W cand = _W_gen();

    p_list_head p_node;
    list_for_each(p_node, &p_bb->prev_branch_target_list) {
        p_ir_basic_block_branch_target p_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        p_ir_basic_block p_source_bb = p_target->p_source_block;
        assert(W_exit_map[p_source_bb->block_id]);
        if (p_node == p_bb->prev_branch_target_list.p_next)
            _W_union(take, W_exit_map[p_source_bb->block_id]);
        else
            _W_insertion(take, W_exit_map[p_source_bb->block_id]);
        _W_union(cand, W_exit_map[p_source_bb->block_id]);
    }
    _W_complement(cand, take);
    _W_sort_by_next_use(take, _get_bb_live_in(p_bb));
    _W_sort_by_next_use(cand, _get_bb_live_in(p_bb));

    printf("must take\n");
    _W_print(take);
    printf("candidate\n");
    _W_print(cand);

    _W_cut(cand, take->vreg_set_r->cnt, take->vreg_set_s->cnt);

    _W_union(take, cand);
    _W_drop(cand);

    printf("choose\n");
    _W_print(take);

    return take;
}

static inline p_W _W_init_loop_header(p_ir_basic_block p_bb) {
    printf("init W entry b%ld for loop bb\n", p_bb->block_id);

    p_W live_through = _W_gen();
    p_W cand = _W_gen();

    p_list_head p_node;
    list_for_each(p_node, &_get_bb_live_in(p_bb)->vreg_list){
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg_list_node, node)->p_vreg;
        p_list_head p_node;
        bool in_loop = false;
        list_for_each(p_node, &p_vreg->use_list) {
            p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
            p_ir_basic_block p_use_bb = NULL;
            switch (p_use->used_type) {
            case instr_ptr:
                p_use_bb = p_use->p_instr->p_basic_block;
                break;
            case bb_param_ptr:
                p_use_bb = p_use->p_bb_param->p_target->p_source_block;
                break;
            case cond_ptr:
            case ret_ptr:
                p_use_bb = p_use->p_basic_block;
                break;
            }
            assert(p_use_bb);
            if (search(p_bb->p_nestree_node->rbtree->root, (uint64_t) p_use_bb)) {
                in_loop = true;
                break;
            }
        }
        if (in_loop)
            _W_add(cand, p_vreg);
        else
            _W_add(live_through, p_vreg);
    }

    _W_sort_by_next_use(cand, _get_bb_live_in(p_bb));
    _W_sort_by_next_use(live_through, _get_bb_live_in(p_bb));

    printf("use in loop\n");
    _W_print(cand);
    printf("live trough\n");
    _W_print(live_through);

    printf("max r register press in loop is %ld\n", p_bb->p_nestree_node->max_reg_pres_r);
    printf("max s register press in loop is %ld\n", p_bb->p_nestree_node->max_reg_pres_s);

    _W_cut(live_through, p_bb->p_nestree_node->max_reg_pres_r - live_through->vreg_set_r->cnt,
            p_bb->p_nestree_node->max_reg_pres_s - live_through->vreg_set_s->cnt);
    _W_cut(cand, 0, 0);

    _W_union(cand, live_through);
    _W_drop(live_through);

    assert(cand->vreg_set_r->cnt <= cand->limit_r);
    assert(cand->vreg_set_s->cnt <= cand->limit_s);

    printf("choose\n");
    _W_print(cand);

    return cand;
}

static inline p_W _S_init(p_W p_w_entry_bb, p_W *W_exit_map, p_W *S_exit_map, p_ir_basic_block p_bb) {
    p_W p_s_entry_bb = _W_gen();

    p_list_head p_node;
    list_for_each(p_node, &p_bb->prev_branch_target_list) {
        p_ir_basic_block_branch_target p_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        p_ir_basic_block p_source_bb = p_target->p_source_block;
        if (p_source_bb->p_nestree_node->head == p_bb)
            continue;

        p_W p_w_exit_p = W_exit_map[p_source_bb->block_id];
        p_W p_complement = _W_copy(p_w_entry_bb);
        _W_complement(p_complement, p_w_exit_p);
        _W_union(p_s_entry_bb, p_complement);
        _W_drop(p_complement);

        p_W p_s_exit_p = S_exit_map[p_source_bb->block_id];
        assert(p_s_exit_p);
        _W_union(p_s_entry_bb, p_s_exit_p);
    }
    _W_insertion(p_s_entry_bb, p_w_entry_bb);
    printf("init S entry b%ld for loop bb\n", p_bb->block_id);
    _W_print(p_s_entry_bb);
    return p_s_entry_bb;
}

static inline void _reload_instr_gen(p_W p_w, p_ir_vreg p_vreg, p_ir_instr p_instr) {
    p_ir_vreg p_copy = ir_vreg_copy(p_vreg);
    symbol_func_vreg_add(p_instr->p_basic_block->p_func, p_copy);
    p_ir_instr p_load = NULL;
    if (p_vreg->def_type == instr_def) {
        p_ir_instr p_def = p_vreg->p_instr_def;
        if (p_def->irkind == ir_unary){
            if (p_def->ir_unary.p_src->kind == imme || _W_find(p_w, p_def->ir_unary.p_src->p_vreg)) {
                p_load = ir_unary_instr_gen(p_def->ir_unary.op, ir_operand_copy(p_def->ir_unary.p_src), p_copy);
            }
        }
        if (p_def->irkind == ir_binary){
            if ((p_def->ir_binary.p_src1->kind == imme || _W_find(p_w, p_def->ir_binary.p_src1->p_vreg))
                    && (p_def->ir_binary.p_src2->kind == imme || _W_find(p_w, p_def->ir_binary.p_src2->p_vreg))) {
                p_load = ir_binary_instr_gen(p_def->ir_binary.op, ir_operand_copy(p_def->ir_binary.p_src1), ir_operand_copy(p_def->ir_binary.p_src2), p_copy);
            }
        }
    }
    if (!p_load) {
        p_load = ir_load_instr_gen(ir_operand_addr_gen(p_vreg->p_info, NULL, 0), p_copy, true);
    }
    ir_instr_add_prev(p_load, p_instr);
    copy_vreg_list(p_load->p_live_in, p_instr->p_live_in);
    copy_vreg_list(p_load->p_live_out, p_instr->p_live_in);
}
static inline void _spill_instr_gen(p_ir_vreg p_vreg, p_ir_instr p_instr) {
    p_ir_instr p_store = ir_store_instr_gen(ir_operand_addr_gen(p_vreg->p_info, NULL, 0), ir_operand_vreg_gen(p_vreg), true);
    ir_instr_add_prev(p_store, p_instr);
    copy_vreg_list(p_store->p_live_in, p_instr->p_live_in);
    copy_vreg_list(p_store->p_live_out, p_instr->p_live_in);
}
static inline void _load_all_before_instr(p_W p_w, p_W need_load, p_ir_instr p_instr) {
    p_list_head p_node;
    list_for_each(p_node, &need_load->vreg_set_r->vregs) {
        p_ir_vreg p_vreg = list_entry(p_node, vreg_set_node, node)->p_vreg;
        _reload_instr_gen(p_w, p_vreg, p_instr);
    }
    list_for_each(p_node, &need_load->vreg_set_s->vregs) {
        p_ir_vreg p_vreg = list_entry(p_node, vreg_set_node, node)->p_vreg;
        _reload_instr_gen(p_w, p_vreg, p_instr);
    }
}
static inline void _spill_all_before_instr(p_W need_spill, p_ir_instr p_instr) {
    p_list_head p_node;
    list_for_each(p_node, &need_spill->vreg_set_r->vregs) {
        p_ir_vreg p_vreg = list_entry(p_node, vreg_set_node, node)->p_vreg;
        _spill_instr_gen(p_vreg, p_instr);
    }
    list_for_each(p_node, &need_spill->vreg_set_s->vregs) {
        p_ir_vreg p_vreg = list_entry(p_node, vreg_set_node, node)->p_vreg;
        _spill_instr_gen(p_vreg, p_instr);
    }
}
static inline void _load_all_at_bb(p_W p_w, p_W need_load, p_ir_basic_block p_bb) {
    p_ir_instr p_instr = ir_call_instr_gen(p_bb->p_func, NULL);
    ir_basic_block_addinstr_tail(p_bb, p_instr);
    copy_vreg_list(p_instr->p_live_in, p_bb->p_branch->p_live_in);
    copy_vreg_list(p_instr->p_live_out, p_bb->p_branch->p_live_in);
    _load_all_before_instr(p_w, need_load, p_instr);
    ir_instr_drop(p_instr);
}
static inline void _spill_all_at_bb(p_W need_spill, p_ir_basic_block p_bb) {
    p_ir_instr p_instr = ir_call_instr_gen(p_bb->p_func, NULL);
    ir_basic_block_addinstr_tail(p_bb, p_instr);
    copy_vreg_list(p_instr->p_live_in, p_bb->p_branch->p_live_in);
    copy_vreg_list(p_instr->p_live_out, p_bb->p_branch->p_live_in);
    _spill_all_before_instr(need_spill, p_instr);
    ir_instr_drop(p_instr);
}
static inline void _add_spill_and_reload_for_edge(p_W *W_entry_map, p_W *S_entry_map, p_W *W_exit_map, p_W *S_exit_map, p_ir_basic_block_branch_target p_target) {
    p_ir_basic_block p_bb = p_target->p_block;
    p_ir_basic_block p_source_bb = p_target->p_source_block;

    p_W p_w_entry_bb = W_entry_map[p_bb->block_id], p_s_entry_bb = S_entry_map[p_bb->block_id];
    p_W p_w_exit_p = W_exit_map[p_source_bb->block_id], p_s_exit_p = S_exit_map[p_source_bb->block_id];
    assert(p_w_entry_bb);
    assert(p_s_entry_bb);
    assert(p_w_exit_p);
    assert(p_s_exit_p);

    p_W before = _W_copy(p_w_exit_p);

    p_W need_load = _W_copy(p_w_entry_bb);
    _W_complement(need_load, p_w_exit_p);

    p_W need_spill = _W_copy(p_w_exit_p);
    _W_complement(need_spill, p_w_entry_bb);
    _W_union(need_spill, p_s_entry_bb);
    _W_complement(need_spill, p_s_exit_p);
    _W_insertion(need_spill, p_w_exit_p);

    _W_sort_by_next_use(need_load, _get_bb_live_in(p_bb));
    _W_sort_by_next_use(need_spill, _get_bb_live_in(p_bb));

    if (p_source_bb->p_branch->kind == ir_cond_branch) {
        assert(p_bb->prev_branch_target_list.p_prev == p_bb->prev_branch_target_list.p_next);
        assert(list_head_alone(&need_load->vreg_set_r->vregs));
        assert(list_head_alone(&need_load->vreg_set_s->vregs));
        assert(list_head_alone(&need_spill->vreg_set_r->vregs));
        assert(list_head_alone(&need_spill->vreg_set_s->vregs));
        _W_drop(before);
        _W_drop(need_load);
        _W_drop(need_spill);
        return;
    }

    printf("add spill and reload for edge b%ld -> b%ld\n", p_source_bb->block_id, p_bb->block_id);

    p_list_head p_node, p_node_src = p_bb->basic_block_phis.p_next;
    list_for_each(p_node, &p_target->block_param) {
        assert(p_node_src != &p_bb->basic_block_phis);
        p_ir_vreg p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param->p_vreg;
        p_ir_vreg p_phi = list_entry(p_node_src, ir_bb_phi, node)->p_bb_phi;
        _W_replace(before, p_phi, p_param);
        _W_replace(need_load, p_phi, p_param);
        _W_replace(need_spill, p_phi, p_param);
        p_node_src = p_node_src->p_next;
    }

    printf("param is {");
    list_for_each(p_node, &p_target->block_param) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_bb_param, node)->p_bb_param->p_vreg;
        printf("%%%ld, ", p_vreg->id);
    }
    printf("}\nphi is {");
    list_for_each(p_node, &p_bb->basic_block_phis) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        printf("%%%ld, ", p_vreg->id);
    }
    printf("}\n");

    printf("w exit b%ld is\n", p_source_bb->block_id);
    _W_print(p_w_exit_p);
    printf("s exit b%ld is\n", p_source_bb->block_id);
    _W_print(p_s_exit_p);
    printf("w entry b%ld is\n", p_bb->block_id);
    _W_print(p_w_entry_bb);
    printf("s entry b%ld is\n", p_bb->block_id);
    _W_print(p_s_entry_bb);

    printf("need add load\n");
    _W_print(need_load);
    printf("need add spill\n");
    _W_print(need_spill);

    _spill_all_at_bb(need_spill, p_source_bb);
    _load_all_at_bb(before, need_load, p_source_bb);
    _W_drop(before);
    _W_drop(need_load);
    _W_drop(need_spill);
}
static inline void _add_spill_and_reload(p_W *W_entry_map, p_W *S_entry_map, p_W *W_exit_map, p_W *S_exit_map, p_ir_basic_block p_bb) {
    p_list_head p_node;
    list_for_each(p_node, &p_bb->prev_branch_target_list) {
        p_ir_basic_block_branch_target p_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        if (p_target->p_source_block->p_nestree_node->head == p_bb)
            continue;
        _add_spill_and_reload_for_edge(W_entry_map, S_entry_map, W_exit_map, S_exit_map, p_target);
    }
}

static inline void _add_to_W(p_W p_w, p_W p_s, p_W need_load, p_ir_vreg p_vreg) {
    if (_W_add(p_w, p_vreg)) {
        _W_add(p_s, p_vreg);
        _W_add(need_load, p_vreg);
        printf("add param %%%ld to W\n", p_vreg->id);
        _W_print(p_w);
        printf("add param %%%ld to S\n", p_vreg->id);
        _W_print(p_s);
    }
}

static inline void _limit(p_W p_w, p_W p_s, p_ir_vreg_list p_live_info, p_ir_instr p_instr, size_t space) {
    _W_sort_by_next_use(p_w, p_live_info);
    _W_sort_by_next_use(p_s, p_live_info);

    if (p_w->vreg_set_r->cnt + space <= p_w->limit_r && p_w->vreg_set_s->cnt + space <= p_w->limit_s)
        return;

    printf("r reg limit to %ld, s reg limit to %ld\n", p_w->limit_r - space, p_w->limit_s - space);

    printf("sort by liveness {");
    p_list_head p_node;
    list_for_each(p_node, &p_live_info->vreg_list) {
        p_ir_vreg_list_node p_vln = list_entry(p_node, ir_vreg_list_node, node);
        printf("%%%ld: %ld, ", p_vln->p_vreg->id, p_vln->next_use);
    }
    printf("}\n");

    printf("W is\n");
    _W_print(p_w);
    printf("S is\n");
    _W_print(p_s);

    p_W spill = _W_gen();
    _W_union(spill, p_w);
    _W_complement(spill, p_s);

    _W_cut(p_w, space, space);
    _W_complement(spill, p_w);

    if (spill->vreg_set_r->cnt || spill->vreg_set_s->cnt) {
        printf("spill vregs are\n");
        _W_print(spill);

        _spill_all_before_instr(spill, p_instr);
    }
    _W_drop(spill);
}

static inline void _MIN_in_bb(p_W p_w, p_W p_s, p_ir_basic_block p_bb) {
    p_list_head p_node;
    list_for_each(p_node, &p_bb->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        printf("deal instr %ld\n", p_instr->instr_id);
        ir_instr_print(p_instr);

        p_ir_operand p_src1 = NULL, p_src2 = NULL;
        p_list_head p_pl = NULL;
        p_ir_vreg p_des = NULL;
        switch (p_instr->irkind) {
        case ir_binary:
            p_src1 = p_instr->ir_binary.p_src1;
            p_src2 = p_instr->ir_binary.p_src2;
            p_des = p_instr->ir_binary.p_des;
            break;
        case ir_unary:
            p_src1 = p_instr->ir_unary.p_src;
            p_des = p_instr->ir_unary.p_des;
            break;
        case ir_load:
            p_src1 = p_instr->ir_load.p_addr;
            p_des = p_instr->ir_load.p_des;
            break;
        case ir_store:
            p_src1 = p_instr->ir_store.p_addr;
            p_src2 = p_instr->ir_store.p_src;
            break;
        case ir_call:
            p_pl = &p_instr->ir_call.param_list;
            p_des = p_instr->ir_call.p_des;
            break;
        case ir_gep:
            assert(0);
            break;
        }
        p_W need_load = _W_gen();
        p_W before = _W_copy(p_w);
        if (p_src1 && p_src1->kind == reg)
            _add_to_W(p_w, p_s, need_load, p_src1->p_vreg);
        if (p_src2 && p_src2->kind == reg)
            _add_to_W(p_w, p_s, need_load, p_src2->p_vreg);
        if (p_pl) {
            p_list_head p_node;
            list_for_each(p_node, p_pl) {
                p_ir_operand p_src = list_entry(p_node, ir_param, node)->p_param;
                if (p_src->kind == reg)
                    _add_to_W(p_w, p_s, need_load, p_src->p_vreg);
            }
        }
        _limit(p_w, p_s, p_instr->p_live_in, p_instr, 0);
        _W_insertion(before, p_w);
        if (p_des && !p_des->if_cond) {
            _limit(p_w, p_s, p_instr->p_live_out, p_instr, 1);
            _W_add(p_w, p_des);
            printf("add des %%%ld to W\n", p_des->id);
            _W_print(p_w);
        }
        if (need_load->vreg_set_r->cnt || need_load->vreg_set_s->cnt) {
            printf("need reload\n");
            _W_print(need_load);
            _load_all_before_instr(before, need_load, p_instr);
        }
        _W_drop(before);
        _W_drop(need_load);
    }
}

void ir_min_spill_func(p_symbol_func p_func) {
    symbol_func_set_block_id(p_func);

    p_spill_slot p_slot = _spill_slot_alloc(p_func);

    p_liveness_info p_live_info = liveness_info_gen(p_func);
    liveness_analysis(p_live_info);
    liveness_info_drop(p_live_info);

    p_W *W_exit_map = malloc(sizeof(p_W) * p_func->block_cnt);
    memset(W_exit_map, 0, sizeof(p_W) * p_func->block_cnt);
    p_W *S_exit_map = malloc(sizeof(p_W) * p_func->block_cnt);
    memset(S_exit_map, 0, sizeof(p_W) * p_func->block_cnt);

    p_W *W_entry_map = malloc(sizeof(p_W) * p_func->block_cnt);
    memset(W_entry_map, 0, sizeof(p_W) * p_func->block_cnt);
    p_W *S_entry_map = malloc(sizeof(p_W) * p_func->block_cnt);
    memset(S_entry_map, 0, sizeof(p_W) * p_func->block_cnt);

    p_list_head bb_head = _rpo_bb_list_gen(p_func);

    p_ir_basic_block p_bb;
    while(p_bb = _bb_rpo_node_pop(bb_head), p_bb) {
        assert(!W_entry_map[p_bb->block_id]);
        assert(!S_entry_map[p_bb->block_id]);
        assert(!W_exit_map[p_bb->block_id]);
        assert(!S_exit_map[p_bb->block_id]);

        p_W p_w, p_s;
        if (p_bb->p_nestree_node->head != p_bb) {
            p_w = _W_init_usual(W_exit_map, p_bb);
        }
        else {
            p_w = _W_init_loop_header(p_bb);
        }
        if (p_bb == p_func->p_entry_block) {
            assert(!p_w->vreg_set_r->cnt);
            assert(!p_w->vreg_set_s->cnt);
            size_t r = 0;
            size_t s = 0;
            p_list_head p_node;
            list_for_each(p_node, &p_func->param_reg_list) {
                p_ir_vreg p_param_vreg = list_entry(p_node, ir_vreg, node);
                if (p_param_vreg->p_type->ref_level || p_param_vreg->p_type->basic != type_f32) {
                    if (++r > R_REG_PARAM_CNT)
                        continue;
                }
                else if (++s > S_REG_PARAM_CNT)
                    continue;
                _W_add(p_w, p_param_vreg);
            }
            printf("in entry block b%ld\n", p_bb->block_id);
            _W_print(p_w);
        }
        W_entry_map[p_bb->block_id] = _W_copy(p_w);

        p_s = _S_init(p_w, W_exit_map, S_exit_map, p_bb);
        S_entry_map[p_bb->block_id] = _W_copy(p_s);

        assert(p_w);
        assert(p_s);

        _add_spill_and_reload(W_entry_map, S_entry_map, W_exit_map, S_exit_map, p_bb);

        _MIN_in_bb(p_w, p_s, p_bb);

        W_exit_map[p_bb->block_id] = p_w;
        S_exit_map[p_bb->block_id] = p_s;

        if (p_bb->p_branch->kind == ir_cond_branch) {
            assert(list_head_alone(&p_bb->p_branch->p_target_1->block_param));
            assert(list_head_alone(&p_bb->p_branch->p_target_2->block_param));
            continue;
        }

        if (p_bb->p_branch->kind == ir_ret_branch) {
            if (!p_bb->p_branch->p_exp)
                continue;
            p_ir_vreg p_vreg = p_bb->p_branch->p_exp->p_vreg;
            if (_W_add(p_w, p_vreg)) {
                _W_add(p_s, p_vreg);
                p_ir_vreg p_copy = ir_vreg_copy(p_vreg);
                symbol_func_vreg_add(p_func, p_copy);
                p_ir_instr p_load = ir_load_instr_gen(ir_operand_addr_gen(p_vreg->p_info, NULL, 0), p_copy, true);
                ir_basic_block_addinstr_tail(p_bb, p_load);
            }
            _W_sort_by_next_use(p_w, p_bb->p_branch->p_live_in);
            assert(p_w->vreg_set_r->cnt <= p_w->limit_r);
            assert(p_w->vreg_set_s->cnt <= p_w->limit_s);
            continue;
        }

        bool W_has_replace = false, S_has_replace = false;
        p_ir_basic_block_branch_target p_target = p_bb->p_branch->p_target_1;
        p_list_head p_node, p_node_src = p_target->p_block->basic_block_phis.p_next;
        list_for_each(p_node, &p_target->block_param) {
            assert(p_node_src != &p_target->p_block->basic_block_phis);
            p_ir_vreg p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param->p_vreg;
            p_ir_vreg p_phi = list_entry(p_node_src, ir_bb_phi, node)->p_bb_phi;
            W_has_replace = _W_replace(p_w, p_param, p_phi);
            S_has_replace = _W_replace(p_s, p_param, p_phi);
            p_node_src = p_node_src->p_next;
        }
        assert(p_node_src == &p_target->p_block->basic_block_phis);
        if (W_has_replace) {
            printf("replace b%ld -> b%ld param to phi in W\n", p_bb->block_id, p_target->p_block->block_id);
            _W_print(p_w);
        }
        if (S_has_replace) {
            printf("replace b%ld -> b%ld param to phi in S\n", p_bb->block_id, p_target->p_block->block_id);
            _W_print(p_s);
        }
        if (ir_basic_block_dom_check(p_bb, p_bb->p_branch->p_target_1->p_block)) {
            _add_spill_and_reload_for_edge(W_entry_map, S_entry_map, W_exit_map, S_exit_map, p_target);
        }
    }
    _rpo_bb_list_drop(bb_head);

    _retain_ssa(p_slot);

    ir_func_deadcode_elimate_just(p_func);

    _spill_slot_drop(p_slot);

    for (size_t i = 0; i < p_func->block_cnt; ++i) {
        assert(W_entry_map[i]);
        assert(S_entry_map[i]);
        assert(W_exit_map[i]);
        assert(S_exit_map[i]);
        _W_drop(W_entry_map[i]);
        _W_drop(S_entry_map[i]);
        _W_drop(W_exit_map[i]);
        _W_drop(S_exit_map[i]);
    }
    free(W_entry_map);
    free(S_entry_map);
    free(W_exit_map);
    free(S_exit_map);

    symbol_func_set_block_id(p_func);
}
