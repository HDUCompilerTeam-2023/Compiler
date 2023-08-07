#include <program/use.h>
#include <program/def.h>
#include <symbol_gen/func.h>
#include <ir.h>
#include <ir/basic_block.h>
#include <ir_gen/instr.h>
#include <ir/vreg.h>
#include <ir/bb_param.h>
#include <ir/param.h>
#include <ir/operand.h>
#include <ir_gen/basic_block.h>
#include <ir_manager/builddomtree.h>
#include <ir_manager/buildnestree.h>
#include <ir_manager/set_cond.h>
#include <ir_opt/deadcode_elimate.h>

typedef struct {
    p_ir_instr p_instr;
    list_head node;
} instr_info, *p_instr_info;

static inline p_instr_info _instr_rpo_node_gen(p_ir_instr p_instr) {
    if (!p_instr)
        return NULL;
    p_instr_info p_info = malloc(sizeof(instr_info));
    *p_info = (instr_info) {
        .p_instr = p_instr,
        .node = list_init_head(&p_info->node),
    };
    return p_info;
}

static inline void _postorder_walk_instr(p_list_head p_head, p_ir_instr p_instr, bool *visit_map) {
    if (visit_map[p_instr->instr_id])
        return;
    visit_map[p_instr->instr_id] = true;

    p_ir_vreg p_des = NULL;
    switch (p_instr->irkind) {
    case ir_binary:
        p_des = p_instr->ir_binary.p_des;
        break;
    case ir_unary:
        p_des = p_instr->ir_unary.p_des;
        break;
    case ir_gep:
        p_des = p_instr->ir_gep.p_des;
        break;
    case ir_call:
        p_des = p_instr->ir_call.p_des;
        break;
    case ir_load:
        p_des = p_instr->ir_load.p_des;
        break;
    case ir_store:
        break;
    }
    if (p_des) {
        p_list_head p_node;
        list_for_each(p_node, &p_des->use_list) {
            p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
            if (p_use->used_type != instr_ptr)
                continue;
            _postorder_walk_instr(p_head, p_use->p_instr, visit_map);
        }
    }

    p_instr_info p_info = _instr_rpo_node_gen(p_instr);
    list_add_next(&p_info->node, p_head);
}

static inline void _init_rpo_list(p_list_head p_head, p_symbol_func p_func) {
    size_t instr_cnt = p_func->instr_num;
    bool *visit_map = malloc(sizeof(bool) * instr_cnt);
    memset(visit_map, 0, sizeof(bool) * instr_cnt);

    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        p_list_head p_node;
        list_for_each(p_node, &p_bb->instr_list) {
            p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
            _postorder_walk_instr(p_head, p_instr, visit_map);
        }
    }

    free(visit_map);
}

static inline bool _is_input(p_ir_instr p_instr, p_ir_instr p_input) {
    assert(p_instr);
    assert(p_input);
    p_ir_vreg p_des = NULL;
    switch (p_input->irkind) {
    case ir_binary:
        p_des = p_input->ir_binary.p_des;
        break;
    case ir_unary:
        p_des = p_input->ir_unary.p_des;
        break;
    case ir_gep:
        p_des = p_input->ir_gep.p_des;
        break;
    case ir_call:
        p_des = p_input->ir_call.p_des;
        break;
    case ir_load:
        p_des = p_input->ir_load.p_des;
        break;
    case ir_store:
        break;
    }
    if (!p_des)
        return false;

    p_ir_operand p_src1 = NULL, p_src2 = NULL;
    p_list_head p_pl = NULL;
    switch (p_instr->irkind) {
    case ir_binary:
        p_src1 = p_instr->ir_binary.p_src1;
        p_src2 = p_instr->ir_binary.p_src2;
        break;
    case ir_unary:
        p_src1 = p_instr->ir_unary.p_src;
        break;
    case ir_gep:
        p_src1 = p_instr->ir_gep.p_addr;
        p_src2 = p_instr->ir_gep.p_offset;
        break;
    case ir_call:
        p_pl = &p_instr->ir_call.param_list;
        break;
    case ir_load:
        p_src1 = p_instr->ir_load.p_addr;
        break;
    case ir_store:
        p_src1 = p_instr->ir_store.p_addr;
        p_src2 = p_instr->ir_store.p_src;
        break;
    }
    if (p_src1 && p_src1->kind == reg && p_src1->p_vreg == p_des)
        return true;
    if (p_src2 && p_src2->kind == reg && p_src2->p_vreg == p_des)
        return true;
    if (!p_pl)
        return false;
    p_list_head p_node;
    list_for_each(p_node, p_pl) {
        p_ir_operand p_src = list_entry(p_node, ir_param, node)->p_param;
        if (p_src && p_src->kind == reg && p_src->p_vreg == p_des)
            return true;
    }
    return false;
}

static inline void _move_instr(p_ir_instr p_instr, p_ir_basic_block p_bb) {
    ir_instr_del(p_instr);
    p_list_head p_node;
    p_ir_instr p_first_use = NULL;
    list_for_each(p_node, &p_bb->instr_list) {
        p_ir_instr p_check = list_entry(p_node, ir_instr, node);
        if (_is_input(p_check, p_instr)) {
            p_first_use = p_check;
            break;
        }
    }
    if (p_first_use) {
        ir_instr_add_prev(p_instr, p_first_use);
        return;
    }
    if (p_bb->p_branch->kind == ir_cond_branch) {
        assert(p_bb->p_branch->p_exp);
        assert(p_bb->p_branch->p_exp->kind == reg);
        assert(p_bb->p_branch->p_exp->p_vreg->def_type == instr_def);
        assert(p_bb->p_branch->p_exp->p_vreg->p_instr_def->p_basic_block == p_bb);
        p_ir_instr p_cond_instr = p_bb->p_branch->p_exp->p_vreg->p_instr_def;
        ir_instr_add_prev(p_instr, p_cond_instr);
        return;
    }
    ir_basic_block_addinstr_tail(p_bb, p_instr);
}

static inline p_ir_basic_block _ir_opt_gcm_schedule_early_deal_src(p_ir_operand p_src, p_ir_basic_block p_early) {
    if (!p_src)
        return p_early;
    if (p_src->kind != reg)
        return p_early;

    p_ir_vreg p_vreg = p_src->p_vreg;
    assert(p_vreg);

    p_ir_basic_block p_def_bb = NULL;
    switch (p_vreg->def_type) {
    case instr_def:
        p_def_bb = p_vreg->p_instr_def->p_basic_block;
        break;
    case bb_phi_def:
        p_def_bb = p_vreg->p_bb_phi->p_basic_block;
        break;
    case func_param_def:
        return p_early;
    }
    assert(p_def_bb);

    if (p_early->dom_depth < p_def_bb->dom_depth) {
        p_early = p_def_bb;
    }
    return p_early;
}

static inline void _ir_opt_gcm_schedule_early(p_ir_instr p_instr) {
    p_ir_operand p_src1 = NULL, p_src2 = NULL;
    p_list_head  p_pl = NULL;
    switch(p_instr->irkind) {
    case ir_binary:
        p_src1 = p_instr->ir_binary.p_src1;
        p_src2 = p_instr->ir_binary.p_src2;
        break;
    case ir_unary:
        p_src1 = p_instr->ir_unary.p_src;
        break;
    case ir_gep:
        p_src1 = p_instr->ir_gep.p_addr;
        p_src2 = p_instr->ir_gep.p_offset;
        break;
    case ir_call:
        p_pl = &p_instr->ir_call.param_list;
        break;
    case ir_load:
        p_src1 = p_instr->ir_load.p_addr;
        break;
    case ir_store:
        p_src1 = p_instr->ir_store.p_addr;
        p_src2 = p_instr->ir_store.p_src;
        break;
    }
    p_ir_basic_block p_early = p_instr->p_basic_block->p_func->p_entry_block;
    p_early = _ir_opt_gcm_schedule_early_deal_src(p_src1, p_early);
    p_early = _ir_opt_gcm_schedule_early_deal_src(p_src2, p_early);
    if (p_pl) {
        p_list_head p_node;
        list_for_each(p_node, p_pl) {
            p_ir_operand p_src = list_entry(p_node, ir_param, node)->p_param;
            p_early = _ir_opt_gcm_schedule_early_deal_src(p_src, p_early);
        }
    }
    if (p_instr->irkind == ir_load || p_instr->irkind == ir_store)
        return;
    if (p_instr->irkind == ir_call && !p_instr->ir_call.p_func->p_side_effects->pure)
        return;
    if (p_instr->irkind == ir_binary && p_instr->ir_binary.p_des->if_cond)
        return;

    _move_instr(p_instr, p_early);
}

static inline p_ir_basic_block _find_lca(p_ir_basic_block p_last_lca, p_ir_basic_block p_bb) {
    if (!p_last_lca)
        return p_bb;
    while (p_last_lca->dom_depth > p_bb->dom_depth)
        p_last_lca = p_last_lca->p_dom_parent;
    while (p_bb->dom_depth > p_last_lca->dom_depth)
        p_bb = p_bb->p_dom_parent;
    while (p_bb != p_last_lca) {
        p_last_lca = p_last_lca->p_dom_parent;
        p_bb = p_bb->p_dom_parent;
    }
    return p_last_lca;
}

static inline void _ir_opt_gcm_schedule_late(p_ir_instr p_instr) {
    p_ir_vreg p_des = NULL;
    switch (p_instr->irkind) {
    case ir_binary:
        p_des = p_instr->ir_binary.p_des;
        break;
    case ir_unary:
        p_des = p_instr->ir_unary.p_des;
        break;
    case ir_gep:
        p_des = p_instr->ir_gep.p_des;
        break;
    case ir_call:
        p_des = p_instr->ir_call.p_des;
        break;
    case ir_load:
        p_des = p_instr->ir_load.p_des;
        break;
    case ir_store:
        break;
    }
    if (!p_des)
        return;

    p_ir_basic_block p_lca = NULL;
    p_list_head p_node;
    list_for_each(p_node, &p_des->use_list) {
        p_ir_operand p_ir_use = list_entry(p_node, ir_operand, use_node);
        assert(p_ir_use->kind == reg && p_ir_use->p_vreg == p_des);
        p_ir_basic_block p_use_bb = NULL;
        switch(p_ir_use->used_type) {
        case bb_param_ptr:
            p_use_bb = p_ir_use->p_bb_param->p_target->p_source_block;
            assert(p_use_bb);
            break;
        case cond_ptr:
        case ret_ptr:
            p_use_bb = p_ir_use->p_basic_block;
            assert(p_use_bb);
            break;
        case instr_ptr:
            p_use_bb = p_ir_use->p_instr->p_basic_block;
            assert(p_use_bb);
            break;
        }
        p_lca = _find_lca(p_lca, p_use_bb);
    }
    if (p_instr->irkind == ir_load || p_instr->irkind == ir_store)
        return;
    if (p_instr->irkind == ir_call && !p_instr->ir_call.p_func->p_side_effects->pure)
        return;
    if (!p_lca)
        return;
    if (p_instr->irkind == ir_binary && p_instr->ir_binary.p_des->if_cond)
        return;

    p_ir_basic_block p_best = p_lca;
    while(p_lca != p_instr->p_basic_block->p_dom_parent) {
        if (p_lca->p_nestree_node->depth < p_best->p_nestree_node->depth)
            p_best = p_lca;
        p_lca = p_lca->p_dom_parent;
    }

    assert(ir_basic_block_dom_check(p_best, p_instr->p_basic_block));
    _move_instr(p_instr, p_best);
}

static inline void _ir_opt_gcm_func(p_symbol_func p_func) {
    symbol_func_set_block_id(p_func);
    list_head rpo_instr_list = list_init_head(&rpo_instr_list);
    _init_rpo_list(&rpo_instr_list, p_func);

    p_list_head p_node;
    list_for_each(p_node, &rpo_instr_list) {
        p_instr_info p_info = list_entry(p_node, instr_info, node);
        _ir_opt_gcm_schedule_early(p_info->p_instr);
    }
    list_for_each_tail(p_node, &rpo_instr_list) {
        p_instr_info p_info = list_entry(p_node, instr_info, node);
        _ir_opt_gcm_schedule_late(p_info->p_instr);
    }

    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &rpo_instr_list) {
        p_instr_info p_info = list_entry(p_node, instr_info, node);
        list_del(&p_info->node);
        free(p_info);
    }
}

void ir_opt_gcm(p_program p_ir) {
    ir_deadcode_elimate_pass(p_ir, false);
    ir_cfg_set_program_dom(p_ir);
    ir_build_program_nestedtree(p_ir);
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        assert(p_func->p_entry_block);
        _ir_opt_gcm_func(p_func);
    }
    ir_cfg_set_program_dom(p_ir);
    ir_build_program_nestedtree(p_ir);
    set_cond_pass(p_ir);
}
