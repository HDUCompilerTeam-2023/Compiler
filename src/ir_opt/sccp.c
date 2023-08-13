#include <ir_manager/builddomtree.h>
#include <ir_manager/buildnestree.h>
#include <ir_manager/set_cond.h>
#include <program/use.h>
#include <program/def.h>
#include <symbol_gen/func.h>
#include <symbol/type.h>
#include <symbol/var.h>
#include <ir/bb_param.h>
#include <ir_gen/vreg.h>
#include <ir_gen/instr.h>
#include <ir_gen/operand.h>
#include <ir_gen/basic_block.h>
#include <ir_opt/deadcode_elimate.h>
#include <ir_opt/const_fold.h>
#include <ir_print.h>
#include <program/print.h>

typedef struct {
    p_ir_basic_block_branch_target p_target;
    list_head node;
} CFG_edge, *p_CFG_edge;

typedef struct {
    p_ir_operand p_use;
    list_head node;
} SSA_edge, *p_SSA_edge;

typedef struct {
    list_head ssa_worklist;
    list_head cfg_worklist;
} WL, *p_WL;

static inline void _add_ssa_edge_to_worklist(p_WL wl, p_ir_vreg p_def) {
    printf("add ssa edge def by %%%ld\n", p_def->id);
    p_list_head p_node;
    list_for_each(p_node, &p_def->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        p_SSA_edge p_new_ssa = malloc(sizeof(SSA_edge));
        *p_new_ssa = (SSA_edge) {
            .p_use = p_use,
            .node = list_init_head(&p_new_ssa->node),
        };
        list_add_prev(&p_new_ssa->node, &wl->ssa_worklist);
    }
}

static inline p_ir_operand _pop_ssa_edge_from_worklist(p_WL p_wl) {
    if (list_head_alone(&p_wl->ssa_worklist))
        return NULL;
    p_list_head p_node = p_wl->ssa_worklist.p_next;
    p_SSA_edge p_edge = list_entry(p_node, SSA_edge, node);
    p_ir_operand p_use = p_edge->p_use;
    list_del(&p_edge->node);
    free(p_edge);
    printf("pop ssa edge def by %%%ld\n", p_use->p_vreg->id);
    return p_use;
}

static inline void _add_cfg_edge_to_worklist(p_WL wl, p_ir_basic_block_branch_target p_target) {
    printf("add cfg edge b%ld -> b%ld\n", p_target->p_source_block->block_id, p_target->p_block->block_id);
    p_CFG_edge p_new_cfg = malloc(sizeof(CFG_edge));
    *p_new_cfg = (CFG_edge) {
        .p_target = p_target,
        .node = list_init_head(&p_new_cfg->node),
    };
    list_add_prev(&p_new_cfg->node, &wl->cfg_worklist);
}

static inline p_ir_basic_block_branch_target _pop_cfg_edge_from_worklist(p_WL p_wl) {
    if (list_head_alone(&p_wl->cfg_worklist))
        return NULL;
    p_list_head p_node = p_wl->cfg_worklist.p_next;
    p_CFG_edge p_edge = list_entry(p_node, CFG_edge, node);
    p_ir_basic_block_branch_target p_target = p_edge->p_target;
    list_del(&p_edge->node);
    free(p_edge);
    return p_target;
}

static inline p_WL _WL_gen(void) {
    p_WL p_wl = malloc(sizeof(WL));
    *p_wl = (WL) {
        .cfg_worklist = list_init_head(&p_wl->cfg_worklist),
        .ssa_worklist = list_init_head(&p_wl->ssa_worklist),
    };
    return p_wl;
}

static inline bool _WL_empty(p_WL p_wl) {
    return list_head_alone(&p_wl->cfg_worklist) && list_head_alone(&p_wl->ssa_worklist);
}

static inline void _WL_drop(p_WL p_wl) {
    assert(list_head_alone(&p_wl->cfg_worklist));
    assert(list_head_alone(&p_wl->ssa_worklist));
    free(p_wl);
}

typedef struct {
    p_ir_vreg p_vreg;
    enum {
        lat_LUB,
        lat_VAL,
        lat_NAC,
    } lat;
    p_ir_operand p_const;
} lattice, *p_lattice;

static inline bool _const_cmp(p_ir_operand p_op1, p_ir_operand p_op2) {
    if (p_op1->p_type->basic != p_op2->p_type->basic)
        return false;
    if (p_op1->p_type->ref_level != p_op2->p_type->ref_level)
        return false;
    if (p_op1->p_type->ref_level)
        return p_op1->p_vmem == p_op2->p_vmem;
    assert(list_head_alone(&p_op1->p_type->array));
    assert(list_head_alone(&p_op2->p_type->array));
    switch (p_op1->p_type->basic) {
    case type_i32:
        return p_op1->i32const == p_op2->i32const;
    case type_f32:
        return p_op1->f32const == p_op2->f32const;
    case type_str:
        return p_op1->strconst == p_op2->strconst;
    case type_void:
        assert(0);
    }
}

static inline bool _set_nac_lattice(const p_lattice p_des) {
    if (p_des->lat == lat_NAC) {
        assert(!p_des->p_const);
        return false;
    }
    printf("%%%ld to NAC\n", p_des->p_vreg->id);
    if (p_des->lat == lat_VAL) {
        assert(p_des->p_const);
        ir_operand_drop(p_des->p_const);
        p_des->p_const = NULL;
    }
    assert(!p_des->p_const);
    p_des->lat = lat_NAC;
    return true;
}

static inline bool _set_val_lattice(const p_lattice p_des, p_ir_operand p_const) {
    assert(p_des->lat != lat_NAC);
    assert(p_const->kind == imme);
    if (p_des->lat == lat_VAL) {
        assert(p_des->p_const);
        assert(p_des->p_const->kind == imme);
        bool same = false;
        printf("cmp ");
        ir_operand_print(p_const);
        printf(" ");
        ir_operand_print(p_des->p_const);
        printf("\n");
        if (_const_cmp(p_const, p_des->p_const))
            same = true;
        ir_operand_drop(p_const);
        if (same)
            return false;
        return _set_nac_lattice(p_des);
    }
    printf("%%%ld to VAL ", p_des->p_vreg->id);
    ir_operand_print(p_const);
    printf("\n");
    assert(!p_des->p_const);
    p_des->p_const = p_const;
    p_des->lat = lat_VAL;
    return true;
}

static inline size_t _get_target_id(p_ir_basic_block_branch_target p_target) {
    size_t id = p_target->p_source_block->block_id << 1;
    assert(p_target->p_source_block->p_branch->p_target_1);
    if (p_target == p_target->p_source_block->p_branch->p_target_1)
        return id;
    assert(p_target->p_source_block->p_branch->p_target_2);
    assert(p_target == p_target->p_source_block->p_branch->p_target_2);
    return id + 1;
}

static inline void _visit_phi(p_ir_basic_block_branch_target p_target, p_lattice lattice_map, p_WL p_wl) {
    printf("visit phi b%ld -> b%ld\n", p_target->p_source_block->block_id, p_target->p_block->block_id);
    p_list_head p_bb_param_list = &p_target->block_param, p_bb_phi_list = &p_target->p_block->basic_block_phis;
    p_list_head p_node, p_node_src = p_bb_param_list->p_next;
    list_for_each(p_node, p_bb_phi_list) {
        assert(p_node_src != p_bb_param_list);
        p_ir_bb_phi p_bb_phi = list_entry(p_node, ir_bb_phi, node);
        assert(p_bb_phi->p_basic_block == p_target->p_block);
        assert(p_bb_phi->p_bb_phi->def_type == bb_phi_def);
        assert(p_bb_phi->p_bb_phi->p_bb_phi == p_bb_phi);

        p_ir_bb_param p_bb_param = list_entry(p_node_src, ir_bb_param, node);
        assert(p_bb_param->p_target == p_target);
        assert(p_bb_param->p_bb_param->used_type == bb_param_ptr);
        assert(p_bb_param->p_bb_param->p_bb_param == p_bb_param);
        assert(p_bb_param->p_bb_param->kind == reg);

        p_node_src = p_node_src->p_next;

        p_ir_vreg p_des = p_bb_phi->p_bb_phi;
        p_ir_vreg p_src = p_bb_param->p_bb_param->p_vreg;

        if (lattice_map[p_des->id].lat == lat_NAC) {
            assert(!lattice_map[p_des->id].p_const);
            continue;
        }
        if (lattice_map[p_src->id].lat == lat_LUB) {
            assert(!lattice_map[p_src->id].p_const);
            continue;
        }
        if (lattice_map[p_src->id].lat == lat_NAC) {
            assert(!lattice_map[p_src->id].p_const);
            if (_set_nac_lattice(lattice_map + p_des->id))
                _add_ssa_edge_to_worklist(p_wl, p_des);
            continue;
        }
        assert(lattice_map[p_src->id].lat == lat_VAL);
        assert(lattice_map[p_src->id].p_const);
        if (_set_val_lattice(lattice_map + p_des->id, ir_operand_copy(lattice_map[p_src->id].p_const)))
            _add_ssa_edge_to_worklist(p_wl, p_des);
    }
    assert(p_node_src == p_bb_param_list);
}

static inline bool _visit_binary(p_ir_binary_instr p_instr, p_lattice lattice_map) {
    p_ir_vreg p_des = p_instr->p_des;
    assert(p_des->def_type == instr_def);
    assert(&p_des->p_instr_def->ir_binary == p_instr);

    p_lattice p_lat_des = lattice_map + p_des->id;

    p_ir_operand p_src1 = p_instr->p_src1, p_src2 = p_instr->p_src2;
    if (p_src1->kind == reg) {
        assert(p_src1->p_vreg);
        if (lattice_map[p_src1->p_vreg->id].lat == lat_LUB)
            return false;
        if (lattice_map[p_src1->p_vreg->id].lat == lat_VAL)
            p_src1 = lattice_map[p_src1->p_vreg->id].p_const;
    }
    assert(p_src1);
    assert(p_src1->kind == imme || lattice_map[p_src1->p_vreg->id].lat == lat_NAC);
    if (p_src2->kind == reg) {
        assert(p_src2->p_vreg);
        if (lattice_map[p_src2->p_vreg->id].lat == lat_LUB)
            return false;
        if (lattice_map[p_src2->p_vreg->id].lat == lat_VAL)
            p_src2 = lattice_map[p_src2->p_vreg->id].p_const;
    }
    assert(p_src2);
    assert(p_src2->kind == imme || lattice_map[p_src2->p_vreg->id].lat == lat_NAC);

    p_ir_operand p_folded = ir_opt_const_fold(p_instr->op, p_src1, p_src2);
    if (p_folded) {
        assert(p_folded->p_type->ref_level == p_instr->p_des->p_type->ref_level);
        assert(p_folded->p_type->basic == p_instr->p_des->p_type->basic);
        return _set_val_lattice(p_lat_des, p_folded);
    }
    return _set_nac_lattice(p_lat_des);
}

static inline bool _visit_unary(p_ir_unary_instr p_instr, p_lattice lattice_map) {
    p_ir_vreg p_des = p_instr->p_des;
    assert(p_des->def_type == instr_def);
    assert(&p_des->p_instr_def->ir_unary == p_instr);

    p_ir_operand p_src = p_instr->p_src;
    if (p_src->kind == reg) {
        assert(p_src->p_vreg);
        if (lattice_map[p_src->p_vreg->id].lat == lat_NAC)
            return _set_nac_lattice(lattice_map + p_des->id);
        if (lattice_map[p_src->p_vreg->id].lat == lat_LUB)
            return false;
        assert(lattice_map[p_src->p_vreg->id].p_const);
        p_src = lattice_map[p_src->p_vreg->id].p_const;
    }
    assert(p_src->kind == imme);

    if (p_instr->op == ir_ptr_add_sp)
        return _set_nac_lattice(lattice_map + p_des->id);

    p_src = ir_operand_copy(p_src);
    switch (p_instr->op) {
    case ir_val_assign:
        break;
    case ir_minus_op:
        assert(p_src->p_type->ref_level == 0);
        assert(list_head_alone(&p_src->p_type->array));
        if (p_src->p_type->basic == type_i32) {
            p_src->i32const = -p_src->i32const;
        }
        else {
            assert(p_src->p_type->basic == type_f32);
            p_src->f32const = -p_src->f32const;
        }
        break;
    case ir_f2i_op:
        assert(p_src->p_type->ref_level == 0);
        assert(list_head_alone(&p_src->p_type->array));
        assert(p_src->p_type->basic == type_f32);
        ir_operand_reset_int(p_src, p_src->f32const);
        break;
    case ir_i2f_op:
        assert(p_src->p_type->ref_level == 0);
        assert(list_head_alone(&p_src->p_type->array));
        assert(p_src->p_type->basic == type_i32);
        ir_operand_reset_float(p_src, p_src->i32const);
        break;
    case ir_ptr_add_sp:
        break;
    }
    return _set_val_lattice(lattice_map + p_des->id, p_src);
}

static inline bool _visit_gep(p_ir_gep_instr p_instr, p_lattice lattice_map) {
    p_ir_vreg p_des = p_instr->p_des;
    assert(p_des->def_type == instr_def);
    assert(&p_des->p_instr_def->ir_gep == p_instr);
    size_t length = symbol_type_get_size(p_des->p_type) * basic_type_get_size(p_des->p_type->basic);

    p_lattice p_lat_des = lattice_map + p_des->id;

    p_ir_operand p_addr = p_instr->p_addr;
    if (p_addr->kind == reg) {
        assert(p_addr->p_vreg);
        if (lattice_map[p_addr->p_vreg->id].lat == lat_LUB)
            return false;
        if (lattice_map[p_addr->p_vreg->id].lat == lat_NAC)
            return _set_nac_lattice(p_lat_des);
        p_addr = lattice_map[p_addr->p_vreg->id].p_const;
    }
    assert(p_addr);
    assert(p_addr->kind == imme);
    assert(p_addr->p_type->ref_level == 1);

    p_ir_operand p_offset = p_instr->p_offset;
    if (p_offset->kind == reg) {
        assert(p_offset->p_vreg);
        if (lattice_map[p_offset->p_vreg->id].lat == lat_LUB)
            return false;
        if (lattice_map[p_offset->p_vreg->id].lat == lat_NAC)
            return _set_nac_lattice(p_lat_des);
        p_offset = lattice_map[p_offset->p_vreg->id].p_const;
    }
    assert(p_offset);
    assert(p_offset->kind == imme);
    assert(p_offset->p_type->ref_level == 0);
    assert(list_head_alone(&p_offset->p_type->array));
    assert(p_offset->p_type->basic == type_i32);

    return _set_val_lattice(p_lat_des, ir_operand_addr_gen(p_addr->p_vmem, p_des->p_type, p_addr->offset + p_offset->i32const * length));
}

static inline void _visit_exp(p_ir_instr p_instr, p_lattice lattice_map, p_WL p_wl) {
    printf("visit exp ");
    ir_instr_print(p_instr);
    p_ir_vreg p_des = NULL;
    bool has_change = false;
    switch (p_instr->irkind) {
    case ir_binary:
        p_des = p_instr->ir_binary.p_des;
        has_change = _visit_binary(&p_instr->ir_binary, lattice_map);
        break;
    case ir_unary:
        p_des = p_instr->ir_unary.p_des;
        has_change = _visit_unary(&p_instr->ir_unary, lattice_map);
        break;
    case ir_gep:
        p_des = p_instr->ir_gep.p_des;
        has_change = _visit_gep(&p_instr->ir_gep, lattice_map);
        break;
    case ir_call:
        p_des = p_instr->ir_call.p_des;
        if (p_des) has_change = _set_nac_lattice(lattice_map + p_des->id);
        break;
    case ir_load:
        p_des = p_instr->ir_load.p_des;
        has_change = _set_nac_lattice(lattice_map + p_des->id);
        break;
    case ir_store:
        break;
    }
    if (!p_des || !has_change)
        return;
    _add_ssa_edge_to_worklist(p_wl, p_des);
}

static inline void _ir_opt_sccp_func(p_symbol_func p_func) {
    p_ir_basic_block p_start = ir_basic_block_gen();
    symbol_func_bb_add_head(p_func, p_start);
    ir_basic_block_set_br(p_start, p_func->p_entry_block);
    p_func->p_entry_block = p_start;
    symbol_func_set_block_id(p_func);

    size_t vreg_cnt = p_func->vreg_cnt + p_func->param_reg_cnt;
    size_t block_cnt = p_func->block_cnt;
    size_t edge_cnt = block_cnt * 2;
    p_lattice lattice_map = malloc(sizeof(lattice) * vreg_cnt);
    bool *block_executed_map = malloc(sizeof(bool) * block_cnt);
    bool *edge_executed_map = malloc(sizeof(bool) * edge_cnt);
    p_list_head p_node;
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        size_t id = p_vreg->id;
        lattice_map[id].p_vreg = p_vreg;
        lattice_map[id].p_const = NULL;
        lattice_map[id].lat = lat_LUB;
    }
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        size_t id = p_vreg->id;
        lattice_map[id].p_vreg = p_vreg;
        lattice_map[id].p_const = NULL;
        lattice_map[id].lat = lat_NAC;
    }
    for (size_t i = 0; i < block_cnt; ++i) {
        block_executed_map[i] = false;
    }
    for (size_t i = 0; i < edge_cnt; ++i) {
        edge_executed_map[i] = false;
    }

    p_WL p_wl = _WL_gen();

    _add_cfg_edge_to_worklist(p_wl, p_start->p_branch->p_target_1);
    while (!_WL_empty(p_wl)) {
        p_ir_operand p_use;
        while(p_use = _pop_ssa_edge_from_worklist(p_wl), p_use) {
            assert(p_use->kind == reg);
            p_ir_vreg p_def = p_use->p_vreg;
            p_lattice p_lat = lattice_map + p_def->id;
            assert(p_lat->lat != lat_LUB);
            if (p_use->used_type == bb_param_ptr) {
                size_t target_id = _get_target_id(p_use->p_bb_param->p_target);
                if (edge_executed_map[target_id])
                    _visit_phi(p_use->p_bb_param->p_target, lattice_map, p_wl);
                continue;
            }
            if (p_use->used_type == instr_ptr) {
                p_ir_instr p_instr = p_use->p_instr;
                size_t block_id = p_instr->p_basic_block->block_id;
                if (block_executed_map[block_id])
                    _visit_exp(p_instr, lattice_map, p_wl);
                continue;
            }
            if (p_use->used_type == cond_ptr) {
                p_ir_basic_block p_block = p_use->p_basic_block;
                assert(p_block->p_branch->kind == ir_cond_branch);
                assert(p_block->p_branch->p_target_1);
                assert(p_block->p_branch->p_target_2);
                assert(p_block->p_branch->p_exp == p_use);
                size_t block_id = p_block->block_id;
                if (!block_executed_map[block_id])
                    continue;
                if (p_lat->lat == lat_NAC) {
                    _add_cfg_edge_to_worklist(p_wl, p_block->p_branch->p_target_1);
                    _add_cfg_edge_to_worklist(p_wl, p_block->p_branch->p_target_2);
                    continue;
                }
                assert(p_lat->lat == lat_VAL);
                assert(p_lat->p_const);
                assert(p_lat->p_const->kind == imme);
                assert(p_lat->p_const->p_type->ref_level == 0);
                assert(list_head_alone(&p_lat->p_const->p_type->array));
                assert(p_lat->p_const->p_type->basic == type_i32);
                I32CONST_t val = p_lat->p_const->i32const;
                if (val) {
                    _add_cfg_edge_to_worklist(p_wl, p_block->p_branch->p_target_1);
                }
                else {
                    _add_cfg_edge_to_worklist(p_wl, p_block->p_branch->p_target_2);
                }
            }
        }
        p_ir_basic_block_branch_target p_target;
        while(p_target = _pop_cfg_edge_from_worklist(p_wl), p_target) {
            p_ir_basic_block p_block = p_target->p_block;

            size_t target_id = _get_target_id(p_target);
            if (edge_executed_map[target_id])
                continue;
            edge_executed_map[target_id] = true;

            printf("deal target b%ld -> b%ld\n", p_target->p_source_block->block_id, p_target->p_block->block_id);
            _visit_phi(p_target, lattice_map, p_wl);

            size_t block_id = p_block->block_id;
            if (block_executed_map[block_id])
                continue;
            block_executed_map[block_id] = true;

            printf("first enter this block\n");
            list_for_each(p_node, &p_block->instr_list) {
                p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
                _visit_exp(p_instr, lattice_map, p_wl);
            }

            if (p_target->p_block->p_branch->kind == ir_br_branch) {
                assert(p_target->p_block->p_branch->p_target_1);
                assert(!p_target->p_block->p_branch->p_target_2);
                _add_cfg_edge_to_worklist(p_wl, p_target->p_block->p_branch->p_target_1);
            }
            else if (p_target->p_block->p_branch->kind == ir_cond_branch) {
                assert(p_block->p_branch->p_target_1);
                assert(p_block->p_branch->p_target_2);
                p_ir_operand p_cond = p_target->p_block->p_branch->p_exp;
                assert(p_cond->kind == reg);
                p_ir_vreg p_def = p_cond->p_vreg;
                p_lattice p_lat = lattice_map + p_def->id;
                if (p_lat->lat == lat_LUB)
                    continue;
                if (p_lat->lat == lat_NAC) {
                    _add_cfg_edge_to_worklist(p_wl, p_block->p_branch->p_target_1);
                    _add_cfg_edge_to_worklist(p_wl, p_block->p_branch->p_target_2);
                    continue;
                }
                assert(p_lat->lat == lat_VAL);
                assert(p_lat->p_const);
                assert(p_lat->p_const->kind == imme);
                assert(p_lat->p_const->p_type->ref_level == 0);
                assert(list_head_alone(&p_lat->p_const->p_type->array));
                assert(p_lat->p_const->p_type->basic == type_i32);
                I32CONST_t val = p_lat->p_const->i32const;
                if (val)
                    _add_cfg_edge_to_worklist(p_wl, p_block->p_branch->p_target_1);
                else
                    _add_cfg_edge_to_worklist(p_wl, p_block->p_branch->p_target_2);
            }
        }
    }

    _WL_drop(p_wl);

    printf("\nsccp finish\n\n");

    for (size_t i = 0; i < vreg_cnt; ++i) {
        p_ir_vreg p_vreg = lattice_map[i].p_vreg;
        switch (lattice_map[i].lat) {
        case lat_LUB:
            switch (p_vreg->def_type) {
            case bb_phi_def:
                assert(!block_executed_map[p_vreg->p_bb_phi->p_basic_block->block_id]);
                break;
            case instr_def:
                assert(!block_executed_map[p_vreg->p_instr_def->p_basic_block->block_id]);
                break;
            case func_param_def:
                assert(0);
            }
            break;
        case lat_NAC:
            break;
        case lat_VAL:
            assert(lattice_map[i].p_const);
            p_ir_operand p_const = lattice_map[i].p_const;
            printf("%%%ld -> ", i);
            ir_operand_print(p_const);
            printf("\n");

            assert(p_const->kind == imme);
            p_ir_vreg p_new = ir_vreg_copy(p_vreg);
            symbol_func_vreg_add(p_func, p_new);
            p_ir_instr p_new_instr = ir_unary_instr_gen(ir_val_assign, ir_operand_copy(p_const), p_new);
            switch (p_vreg->def_type) {
            case bb_phi_def:
                ir_basic_block_addinstr_head(p_vreg->p_bb_phi->p_basic_block, p_new_instr);
                break;
            case instr_def:
                ir_instr_add_next(p_new_instr, p_vreg->p_instr_def);
                break;
            case func_param_def:
                assert(0);
            }

            p_list_head p_node, p_next;
            list_for_each_safe(p_node, p_next, &p_vreg->use_list) {
                p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
                assert(p_use->kind == reg);
                assert(p_use->p_vreg == p_vreg);
                switch (p_use->used_type) {
                case cond_ptr:
                    assert(p_const->p_type->ref_level == 0);
                    assert(list_head_alone(&p_const->p_type->array));
                    assert(p_const->p_type->basic == type_i32);

                    p_ir_basic_block p_bb = p_use->p_basic_block;
                    assert(p_bb->p_branch->p_exp == p_use);
                    assert(p_bb->p_branch->kind == ir_cond_branch);
                    assert(p_bb->p_branch->p_target_1 && p_bb->p_branch->p_target_2);

                    bool edge_executed_1 = edge_executed_map[p_bb->block_id << 1], edge_executed_2 = edge_executed_map[(p_bb->block_id << 1) + 1];
                    assert(edge_executed_1 ^ edge_executed_2);
                    if (edge_executed_2) {
                        assert(!p_const->i32const);
                        p_ir_basic_block_branch_target p_tmp = p_bb->p_branch->p_target_1;
                        p_bb->p_branch->p_target_1 = p_bb->p_branch->p_target_2;
                        p_bb->p_branch->p_target_2 = p_tmp;
                    }
                    else {
                        assert(p_const->i32const);
                    }
                    ir_basic_block_branch_target_drop(p_bb, p_bb->p_branch->p_target_2);
                    p_bb->p_branch->p_target_2 = NULL;

                    ir_operand_drop(p_bb->p_branch->p_exp);
                    p_bb->p_branch->p_exp = NULL;

                    p_bb->p_branch->kind = ir_br_branch;
                    break;
                case instr_ptr:
                    ir_operand_reset_operand(p_use, p_const);
                    break;
                case bb_param_ptr:
                    ir_operand_reset_vreg(p_use, p_new);
                    break;
                case ret_ptr:
                    ir_operand_reset_vreg(p_use, p_new);
                    break;
                }
            }
            assert(list_head_alone(&p_vreg->use_list));
            ir_operand_drop(p_const);
            break;
        }
    }

    free(lattice_map);
    free(edge_executed_map);
    free(block_executed_map);
}

void ir_opt_sccp(p_program p_ir) {
    ir_cfg_set_program_dom(p_ir);
    ir_build_program_nestedtree(p_ir);
    set_cond_pass(p_ir);
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        assert(p_func->p_entry_block);
        _ir_opt_sccp_func(p_func);
    }
    ir_deadcode_elimate_pass(p_ir, true);
    ir_cfg_set_program_dom(p_ir);
    ir_build_program_nestedtree(p_ir);
    set_cond_pass(p_ir);
}

