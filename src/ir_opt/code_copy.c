#include <ir_gen/basic_block.h>
#include <ir_gen/vreg.h>
#include <ir_gen/instr.h>
#include <ir_gen/operand.h>
#include <ir/bb_param.h>
#include <ir/param.h>
#include <symbol_gen/var.h>
#include <symbol_gen/type.h>
#include <symbol/func.h>
#include <ir_opt/code_copy.h>
#include <symbol_gen/func.h>

#define hash_P 83

#define reg_hash_MOD 122777
#define bb_hash_MOD 211
#define var_hash_MOD 211

struct copy_map {
    hlist_hash reg_map, bb_map, var_map;
};

typedef struct {
    p_ir_vreg p_key, p_val;
    hlist_node node;
} reg_map, *p_reg_map;

typedef struct {
    p_ir_basic_block p_key, p_val;
    hlist_node node;
} bb_map, *p_bb_map;

typedef struct {
    p_symbol_var p_key, p_val;
    hlist_node node;
} var_map, *p_var_map;

p_copy_map ir_code_copy_map_gen(void) {
    p_copy_map p_map = malloc(sizeof(*p_map));
    *p_map = (copy_map) {
        .reg_map = malloc(sizeof(*p_map->reg_map) * reg_hash_MOD),
        .bb_map = malloc(sizeof(*p_map->bb_map) * bb_hash_MOD),
        .var_map = malloc(sizeof(*p_map->var_map) * var_hash_MOD),
    };
    for (size_t i = 0; i < reg_hash_MOD; ++i) {
        *(p_map->reg_map + i) = hlist_init_head;
    }
    for (size_t i = 0; i < var_hash_MOD; ++i) {
        *(p_map->var_map + i) = hlist_init_head;
    }
    for (size_t i = 0; i < bb_hash_MOD; ++i) {
        *(p_map->bb_map + i) = hlist_init_head;
    }
    return p_map;
}

void ir_code_copy_map_drop(p_copy_map p_map) {
    for (size_t i = 0; i < reg_hash_MOD; ++i) {
        while(!hlist_head_empty(p_map->reg_map + i)) {
            p_reg_map p_rm = hlist_entry((p_map->reg_map + i)->p_first, reg_map, node);
            hlist_node_del(&p_rm->node);
            free(p_rm);
        }
    }
    for (size_t i = 0; i < bb_hash_MOD; ++i) {
        while(!hlist_head_empty(p_map->bb_map + i)) {
            p_bb_map p_bm = hlist_entry((p_map->bb_map + i)->p_first, bb_map, node);
            hlist_node_del(&p_bm->node);
            free(p_bm);
        }
    }
    for (size_t i = 0; i < var_hash_MOD; ++i) {
        while(!hlist_head_empty(p_map->var_map + i)) {
            p_var_map p_vm = hlist_entry((p_map->var_map + i)->p_first, var_map, node);
            hlist_node_del(&p_vm->node);
            free(p_vm);
        }
    }
    free(p_map->reg_map);
    free(p_map->var_map);
    free(p_map->bb_map);
    free(p_map);
}

static inline void _copy_map_add_reg(p_ir_vreg p_key, p_ir_vreg p_val, p_copy_map p_map) {
    size_t hash_tag = ((size_t) p_key * hash_P) % reg_hash_MOD;
    p_hlist_head p_head = p_map->reg_map + hash_tag;
    p_hlist_node p_node;
    hlist_for_each(p_node, p_head) {
        p_reg_map p_rm = hlist_entry(p_node, reg_map, node);
        assert(p_rm->p_key != p_key);
    }
    p_reg_map p_rm = malloc(sizeof(*p_rm));
    *p_rm = (reg_map) {
        .p_key = p_key,
        .p_val = p_val,
        .node = hlist_init_node,
    };
    hlist_node_add(p_head, &p_rm->node);
}

static inline void _copy_map_add_bb(p_ir_basic_block p_key, p_ir_basic_block p_val, p_copy_map p_map) {
    size_t hash_tag = ((size_t) p_key * hash_P) % bb_hash_MOD;
    p_hlist_head p_head = p_map->bb_map + hash_tag;
    p_hlist_node p_node;
    hlist_for_each(p_node, p_head) {
        printf("1\n");
        p_bb_map p_bm = hlist_entry(p_node, bb_map, node);
        assert(p_bm->p_key != p_key);
    }
    p_bb_map p_bm = malloc(sizeof(*p_bm));
    *p_bm = (bb_map) {
        .p_key = p_key,
        .p_val = p_val,
        .node = hlist_init_node,
    };
    hlist_node_add(p_head, &p_bm->node);
}

static inline void _copy_map_add_var(p_symbol_var p_key, p_symbol_var p_val, p_copy_map p_map) {
    size_t hash_tag = ((size_t) p_key * hash_P) % var_hash_MOD;
    p_hlist_head p_head = p_map->var_map + hash_tag;
    p_hlist_node p_node;
    hlist_for_each(p_node, p_head) {
        p_var_map p_vm = hlist_entry(p_node, var_map, node);
        assert(p_vm->p_key != p_key);
    }
    p_var_map p_vm = malloc(sizeof(*p_vm));
    *p_vm = (var_map) {
        .p_key = p_key,
        .p_val = p_val,
        .node = hlist_init_node,
    };
    hlist_node_add(p_head, &p_vm->node);
}

static inline p_ir_vreg _copy_map_get_reg(p_ir_vreg p_key, p_copy_map p_map) {
    if (!p_key)
        return NULL;
    size_t hash_tag = ((size_t) p_key * hash_P) % reg_hash_MOD;
    p_hlist_head p_head = p_map->reg_map + hash_tag;
    p_hlist_node p_node;
    hlist_for_each(p_node, p_head) {
        p_reg_map p_rm = hlist_entry(p_node, reg_map, node);
        if (p_rm->p_key == p_key)
            return p_rm->p_val;
    }
    return p_key;
    assert(0);
}

static inline p_ir_basic_block _copy_map_get_bb(p_ir_basic_block p_key, p_copy_map p_map) {
    assert(p_key);
    size_t hash_tag = ((size_t) p_key * hash_P) % bb_hash_MOD;
    p_hlist_head p_head = p_map->bb_map + hash_tag;
    p_hlist_node p_node;
    hlist_for_each(p_node, p_head) {
        p_bb_map p_bm = hlist_entry(p_node, bb_map, node);
        if (p_bm->p_key == p_key)
            return p_bm->p_val;
    }
    return p_key;
    assert(0);
}

static inline p_symbol_var _copy_map_get_var(p_symbol_var p_key, p_copy_map p_map) {
    assert(p_key);
    size_t hash_tag = ((size_t) p_key * hash_P) % var_hash_MOD;
    p_hlist_head p_head = p_map->var_map + hash_tag;
    p_hlist_node p_node;
    hlist_for_each(p_node, p_head) {
        p_var_map p_vm = hlist_entry(p_node, var_map, node);
        if (p_vm->p_key == p_key)
            return p_vm->p_val;
    }
    return p_key;
    assert(0);
}

static inline p_ir_operand _copy_map_get_operand(p_ir_operand p_operand, p_copy_map p_map) {
    if (!p_operand)
        return NULL;
    if (p_operand->kind == reg) {
        p_ir_vreg p_vreg_copy = _copy_map_get_reg(p_operand->p_vreg, p_map);
        return ir_operand_vreg_gen(p_vreg_copy);
    }
    if (p_operand->p_type->ref_level == 0)
        return ir_operand_copy(p_operand);
    if (p_operand->p_vmem->is_global)
        return ir_operand_copy(p_operand);
    p_symbol_var p_var_copy = _copy_map_get_var(p_operand->p_vmem, p_map);
    return ir_operand_addr_gen(p_var_copy, p_operand->p_type, p_operand->offset);
}

static inline p_ir_instr _copy_map_get_instr(p_ir_instr p_instr, p_copy_map p_map) {
    assert(p_instr);
    switch(p_instr->irkind) {
    case ir_binary:
        return ir_binary_instr_gen(p_instr->ir_binary.op,
                _copy_map_get_operand(p_instr->ir_binary.p_src1, p_map),
                _copy_map_get_operand(p_instr->ir_binary.p_src2, p_map),
                _copy_map_get_reg(p_instr->ir_binary.p_des, p_map));
    case ir_unary:
        return ir_unary_instr_gen(p_instr->ir_unary.op,
                _copy_map_get_operand(p_instr->ir_unary.p_src, p_map),
                _copy_map_get_reg(p_instr->ir_unary.p_des, p_map));
    case ir_gep:
        return ir_gep_instr_gen(_copy_map_get_operand(p_instr->ir_gep.p_addr, p_map),
                _copy_map_get_operand(p_instr->ir_gep.p_offset, p_map),
                _copy_map_get_reg(p_instr->ir_gep.p_des, p_map),
                p_instr->ir_gep.is_element);
    case ir_load:
        return ir_load_instr_gen(_copy_map_get_operand(p_instr->ir_load.p_addr, p_map),
                _copy_map_get_reg(p_instr->ir_load.p_des, p_map),
                p_instr->ir_load.is_stack_ptr);
    case ir_store:
        return ir_store_instr_gen(_copy_map_get_operand(p_instr->ir_store.p_addr, p_map),
                _copy_map_get_operand(p_instr->ir_store.p_src, p_map),
                p_instr->ir_store.is_stack_ptr);
    case ir_call:
        break;
    }
    p_ir_instr p_call_copy = ir_call_instr_gen(p_instr->ir_call.p_func, _copy_map_get_reg(p_instr->ir_call.p_des, p_map));
    p_list_head p_node;
    list_for_each(p_node, &p_instr->ir_call.param_list) {
        p_ir_param p_param = list_entry(p_node, ir_param, node);
        ir_call_param_list_add(p_call_copy, _copy_map_get_operand(p_param->p_param, p_map));
    }
    return p_call_copy;
}

p_ir_vreg ir_code_copy_vreg(p_ir_vreg p_vreg, p_copy_map p_map) {
    assert(p_vreg);
    p_ir_vreg p_vreg_copy = ir_vreg_copy(p_vreg);
    _copy_map_add_reg(p_vreg, p_vreg_copy, p_map);
    return p_vreg_copy;
}

p_symbol_var ir_code_copy_var(p_symbol_var p_var, p_copy_map p_map) {
    assert(p_var);

    p_symbol_var p_var_copy;
    if (!p_var->name) {
        assert(!p_var->is_const);
        p_var_copy = symbol_temp_var_gen(symbol_type_copy(p_var->p_type));
    }
    else {
        p_var_copy = symbol_var_gen(p_var->name, symbol_type_copy(p_var->p_type), p_var->is_const, NULL);
    }
    _copy_map_add_var(p_var, p_var_copy, p_map);
    return p_var_copy;
}

p_ir_basic_block ir_code_copy_bb(p_ir_basic_block p_bb, p_copy_map p_map) {
    assert(p_bb);
    p_ir_basic_block p_bb_copy = ir_basic_block_gen();
    _copy_map_add_bb(p_bb, p_bb_copy, p_map);
    return p_bb_copy;
}

static inline void _ir_code_copy_instr_of_block_instr(p_ir_basic_block p_des, p_ir_basic_block p_src, p_copy_map p_map) {
    assert(p_src);
    p_list_head p_node;
    list_for_each(p_node, &p_src->basic_block_phis) {
        p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
        assert(p_phi->p_basic_block == p_src);
        ir_basic_block_add_phi(p_des, _copy_map_get_reg(p_phi->p_bb_phi, p_map));
    }
    list_for_each(p_node, &p_src->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        p_ir_instr p_instr_copy = _copy_map_get_instr(p_instr, p_map);
        ir_basic_block_addinstr_tail(p_des, p_instr_copy);
        if (p_instr->irkind == ir_call)
            ir_call_instr_node_gen(p_instr_copy);
    }
}

static inline void _ir_code_copy_instr_of_block_br_branch(p_ir_basic_block p_des, p_ir_basic_block p_src, p_copy_map p_map) {
    ir_basic_block_set_br(p_des, _copy_map_get_bb(p_src->p_branch->p_target_1->p_block, p_map));
    p_list_head p_node;
    list_for_each(p_node, &p_src->p_branch->p_target_1->block_param) {
        p_ir_bb_param p_param = list_entry(p_node, ir_bb_param, node);
        assert(p_param->p_target == p_src->p_branch->p_target_1);
        ir_basic_block_branch_target_add_param(p_des->p_branch->p_target_1, _copy_map_get_operand(p_param->p_bb_param, p_map));
    }
}

static inline void _ir_code_copy_instr_of_block_cond_branch(p_ir_basic_block p_des, p_ir_basic_block p_src, p_copy_map p_map) {
        ir_basic_block_set_cond(p_des,
                _copy_map_get_operand(p_src->p_branch->p_exp, p_map),
                _copy_map_get_bb(p_src->p_branch->p_target_1->p_block, p_map),
                _copy_map_get_bb(p_src->p_branch->p_target_2->p_block, p_map));
        p_list_head p_node;
        list_for_each(p_node, &p_src->p_branch->p_target_1->block_param) {
            p_ir_bb_param p_param = list_entry(p_node, ir_bb_param, node);
            assert(p_param->p_target == p_src->p_branch->p_target_1);
            ir_basic_block_branch_target_add_param(p_des->p_branch->p_target_1, _copy_map_get_operand(p_param->p_bb_param, p_map));
        }
        list_for_each(p_node, &p_src->p_branch->p_target_2->block_param) {
            p_ir_bb_param p_param = list_entry(p_node, ir_bb_param, node);
            assert(p_param->p_target == p_src->p_branch->p_target_2);
            ir_basic_block_branch_target_add_param(p_des->p_branch->p_target_2, _copy_map_get_operand(p_param->p_bb_param, p_map));
        }

}

void ir_code_copy_instr_of_block(p_ir_basic_block p_src, p_copy_map p_map) {
    p_ir_basic_block p_des = _copy_map_get_bb(p_src, p_map);
    _ir_code_copy_instr_of_block_instr(p_des, p_src, p_map);
    switch (p_src->p_branch->kind) {
    case ir_br_branch:
        _ir_code_copy_instr_of_block_br_branch(p_des, p_src, p_map);
        break;
    case ir_cond_branch:
        _ir_code_copy_instr_of_block_cond_branch(p_des, p_src, p_map);
        break;
    case ir_ret_branch:
        ir_basic_block_set_ret(p_des, _copy_map_get_operand(p_src->p_branch->p_exp, p_map));
        break;
    case ir_abort_branch:
        break;
    }
}

void ir_code_copy_instr_of_block_inline(p_ir_basic_block p_src, p_copy_map p_map, p_ir_instr p_call, p_ir_basic_block p_next) {
    p_ir_basic_block p_des = _copy_map_get_bb(p_src, p_map);
    _ir_code_copy_instr_of_block_instr(p_des, p_src, p_map);
    switch (p_src->p_branch->kind) {
    case ir_br_branch:
        _ir_code_copy_instr_of_block_br_branch(p_des, p_src, p_map);
        break;
    case ir_cond_branch:
        _ir_code_copy_instr_of_block_cond_branch(p_des, p_src, p_map);
        break;
    case ir_ret_branch:
        assert(p_src == p_src->p_func->p_ret_block);
        assert(p_call->irkind == ir_call);
        ir_basic_block_set_br(p_des, p_next);
        if (p_call->ir_call.p_des) {
            assert(p_src->p_branch->p_exp);
            ir_instr_reset_unary(p_call, ir_val_assign, _copy_map_get_operand(p_src->p_branch->p_exp, p_map), p_call->ir_call.p_des);
            break;
        }
        ir_instr_drop(p_call);
        break;
    case ir_abort_branch:
        break;
    }
}

void loop_block_vreg_copy(p_ir_basic_block p_block, p_copy_map p_map) {
    p_list_head p_node;
    list_for_each(p_node, &p_block->basic_block_phis) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        p_ir_vreg p_vreg_copy = ir_code_copy_vreg(p_vreg, p_map);
        symbol_func_vreg_add(p_block->p_func, p_vreg_copy);
    }
    list_for_each(p_node, &p_block->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        p_ir_vreg p_vreg = NULL;
        switch (p_instr->irkind) {
        case ir_binary:
            p_vreg = p_instr->ir_binary.p_des;
            break;
        case ir_unary:
            p_vreg = p_instr->ir_unary.p_des;
            break;
        case ir_gep:
            p_vreg = p_instr->ir_gep.p_des;
            break;
        case ir_load:
            p_vreg = p_instr->ir_load.p_des;
            break;
        case ir_call:
            p_vreg = p_instr->ir_call.p_des;
            break;
        default:
            break;
        }
        if (!p_vreg) continue;
        p_ir_vreg p_vreg_copy = ir_code_copy_vreg(p_vreg, p_map);
        symbol_func_vreg_add(p_block->p_func, p_vreg_copy);
    }
}