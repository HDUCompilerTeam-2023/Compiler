#include <program/use.h>
#include <program/def.h>
#include <symbol_gen/func.h>
#include <symbol_gen/type.h>
#include <ir/vreg.h>
#include <ir_gen/operand.h>
#include <ir/bb_param.h>
#include <ir/param.h>
#include <ir/basic_block.h>
#include <ir_gen/instr.h>
#include <ir_opt/const_fold.h>
#include <ir_opt/deadcode_elimate.h>

#define hash_SRC  12281
#define hash_OP  69061
#define hash_MOD 122777

typedef struct {
    p_ir_instr p_instr;
    hlist_node node;
} instr_hash_node, *p_instr_hash_node;

static inline size_t _operand_hash_tag(p_ir_operand p_src) {
    if (!p_src)
        return 0;
    size_t src_val = 0;
    if (p_src->kind == reg)
        src_val = (size_t) p_src->p_vreg;
    else if (p_src->p_type->ref_level > 0)
        src_val = (size_t) p_src->p_vmem + p_src->offset;
    else
        src_val = p_src->i32const;

    src_val %= hash_MOD;
    src_val *= hash_SRC;
    src_val %= hash_MOD;
    return src_val;
}

// op * hash_OP + sum(src * hash_SRC)
static inline size_t _exp_hash_tag(p_ir_instr p_instr) {
    p_ir_operand p_src1 = NULL, p_src2 = NULL;
    size_t hash = 0;
    switch (p_instr->irkind) {
    case ir_binary:
        p_src1 = p_instr->ir_binary.p_src1;
        p_src2 = p_instr->ir_binary.p_src2;
        hash = (p_instr->ir_binary.op * hash_OP) % hash_MOD; // 11 op
        break;
    case ir_unary:
        p_src1 = p_instr->ir_unary.p_src;
        hash = ((p_instr->ir_unary.op + 17) * hash_OP) % hash_MOD; // 5 op
        break;
    case ir_gep:
        p_src1 = p_instr->ir_gep.p_addr;
        p_src2 = p_instr->ir_gep.p_offset;
        hash = (23 * hash_OP) % hash_MOD;
        break;
    case ir_call:
    case ir_load:
    case ir_store:
        break;
    }

    hash += _operand_hash_tag(p_src1);
    hash %= hash_MOD;
    hash += _operand_hash_tag(p_src2);
    hash %= hash_MOD;
    return hash;
}

static inline bool _operand_cmp(p_ir_operand p_operand, p_ir_operand p_cmp) {
    if (p_operand->kind != p_cmp->kind)
        return false;

    if (p_operand->kind == reg)
        return p_operand->p_vreg == p_cmp->p_vreg;

    // type
    if (p_operand->p_type->ref_level != p_cmp->p_type->ref_level)
        return false;
    if (p_operand->p_type->basic != p_cmp->p_type->basic)
        return false;

    // imme
    if (!p_operand->p_type->ref_level) {
        assert(list_head_alone(&p_operand->p_type->array));
        assert(list_head_alone(&p_cmp->p_type->array));
        if (p_operand->p_type->basic == type_i32)
            return p_operand->i32const == p_cmp->i32const;
        if (p_operand->p_type->basic == type_f32)
            return p_operand->f32const == p_cmp->f32const;
        if (p_operand->p_type->basic == type_str)
            return p_operand->strconst == p_cmp->strconst;
        // type_void
        assert(0);
    }

    // vmem
    if (p_operand->p_vmem != p_cmp->p_vmem)
        return false;

    p_list_head p_node, p_node_cmp = p_cmp->p_type->array.p_next;
    list_for_each(p_node, &p_operand->p_type->array) {
        if (p_node_cmp == &p_cmp->p_type->array)
            return false;
        p_symbol_type_array p_array = list_entry(p_node, symbol_type_array, node);
        p_symbol_type_array p_array_cmp = list_entry(p_node_cmp, symbol_type_array, node);
        if (p_array->size != p_array_cmp->size)
            return false;
        p_node_cmp = p_node_cmp->p_next;
    }
    return p_node_cmp == &p_cmp->p_type->array;
}

static inline bool _instr_cmp_binary(p_ir_binary_instr p_instr, p_ir_binary_instr p_cmp) {
    switch (p_instr->op) {
    case ir_add_op:
    case ir_mul_op:
    case ir_eq_op:
    case ir_neq_op:
        if (p_instr->op != p_cmp->op)
            return false;
        if (_operand_cmp(p_instr->p_src1, p_cmp->p_src1))
            return _operand_cmp(p_instr->p_src1, p_cmp->p_src1);
        return _operand_cmp(p_instr->p_src1, p_cmp->p_src2) && _operand_cmp(p_instr->p_src2, p_cmp->p_src1);
    case ir_sub_op:
    case ir_div_op:
    case ir_mod_op:
        if (p_instr->op != p_cmp->op)
            return false;
        return _operand_cmp(p_instr->p_src1, p_cmp->p_src1) && _operand_cmp(p_instr->p_src2, p_cmp->p_src2);
    case ir_g_op:
        if (p_cmp->op == ir_l_op)
            return _operand_cmp(p_instr->p_src1, p_cmp->p_src2) && _operand_cmp(p_instr->p_src2, p_cmp->p_src1);
        return _operand_cmp(p_instr->p_src1, p_cmp->p_src1) && _operand_cmp(p_instr->p_src2, p_cmp->p_src2);
    case ir_l_op:
        if (p_cmp->op == ir_g_op)
            return _operand_cmp(p_instr->p_src1, p_cmp->p_src2) && _operand_cmp(p_instr->p_src2, p_cmp->p_src1);
        return _operand_cmp(p_instr->p_src1, p_cmp->p_src1) && _operand_cmp(p_instr->p_src2, p_cmp->p_src2);
    case ir_geq_op:
        if (p_cmp->op == ir_leq_op)
            return _operand_cmp(p_instr->p_src1, p_cmp->p_src2) && _operand_cmp(p_instr->p_src2, p_cmp->p_src1);
        return _operand_cmp(p_instr->p_src1, p_cmp->p_src1) && _operand_cmp(p_instr->p_src2, p_cmp->p_src2);
    case ir_leq_op:
        if (p_cmp->op == ir_geq_op)
            return _operand_cmp(p_instr->p_src1, p_cmp->p_src2) && _operand_cmp(p_instr->p_src2, p_cmp->p_src1);
        return _operand_cmp(p_instr->p_src1, p_cmp->p_src1) && _operand_cmp(p_instr->p_src2, p_cmp->p_src2);
    }
}

static inline bool _instr_cmp_unary(p_ir_unary_instr p_instr, p_ir_unary_instr p_cmp) {
    if (p_instr->op != p_cmp->op)
        return false;
    return _operand_cmp(p_instr->p_src, p_cmp->p_src);
}

static inline bool _instr_cmp_gep(p_ir_gep_instr p_instr, p_ir_gep_instr p_cmp) {
    if (p_instr->is_element != p_cmp->is_element)
        return false;
    return _operand_cmp(p_instr->p_addr, p_cmp->p_addr) && _operand_cmp(p_instr->p_offset, p_cmp->p_offset);
}

static inline p_ir_vreg _hash_find(p_ir_instr p_instr, hlist_hash instr_hash) {
    size_t hash_tag = _exp_hash_tag(p_instr);
    p_hlist_head p_head = instr_hash + hash_tag;

    p_hlist_node p_node;
    hlist_for_each(p_node, p_head) {
        p_instr_hash_node p_hn = hlist_entry(p_node, instr_hash_node, node);
        p_ir_instr p_find = p_hn->p_instr;
        if (p_find->irkind != p_instr->irkind)
            continue;
        switch (p_find->irkind) {
        case ir_binary:
            if (_instr_cmp_binary(&p_instr->ir_binary, &p_find->ir_binary)) {
                printf("find sim instr %ld and %ld\n", p_instr->instr_id, p_find->instr_id);
                return p_find->ir_binary.p_des;
            }
            break;
        case ir_unary:
            if (_instr_cmp_unary(&p_instr->ir_unary, &p_find->ir_unary)) {
                printf("find sim instr %ld and %ld\n", p_instr->instr_id, p_find->instr_id);
                return p_find->ir_unary.p_des;
            }
            break;
        case ir_gep:
            if (_instr_cmp_gep(&p_instr->ir_gep, &p_find->ir_gep)) {
                printf("find sim instr %ld and %ld\n", p_instr->instr_id, p_find->instr_id);
                return p_find->ir_gep.p_des;
            }
            break;
        case ir_call:
        case ir_load:
        case ir_store:
            assert(0);
        }
    }

    p_instr_hash_node p_hash_node = malloc(sizeof(*p_hash_node));
    *p_hash_node = (instr_hash_node) {
        .p_instr = p_instr,
        .node = hlist_init_node,
    };

    hlist_node_add(p_head, &p_hash_node->node);

    return NULL;
}

static inline p_ir_bb_phi _find_bb_phi_for_bb_param(p_ir_bb_param p_bb_param) {
    p_list_head p_node, p_node_src = p_bb_param->p_target->block_param.p_next;
    list_for_each(p_node, &p_bb_param->p_target->p_block->basic_block_phis) {
        assert(p_node_src != &p_bb_param->p_target->block_param);
        p_ir_bb_param p_param = list_entry(p_node_src, ir_bb_param, node);
        p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
        if (p_param == p_bb_param)
            return p_phi;
        p_node_src = p_node_src->p_next;
    }
    assert(0);
}

typedef struct {
    p_ir_vreg p_vreg;
    list_head node;
} vreg_rpo_node, *p_vreg_rpo_node;

static inline p_vreg_rpo_node _vreg_rpo_node_gen(p_ir_vreg p_vreg) {
    if (!p_vreg)
        return NULL;
    p_vreg_rpo_node p_node = malloc(sizeof(vreg_rpo_node));
    *p_node = (vreg_rpo_node) {
        .p_vreg = p_vreg,
        .node = list_init_head(&p_node->node),
    };
    return p_node;
}

static inline void _postorder_walk_vreg(p_list_head p_head, p_ir_vreg pvreg, bool *visit_map);

static inline void _postorder_walk_instr(p_list_head p_head, p_ir_instr p_instr, bool *visit_map) {
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
    if (visit_map[p_des->id])
        return;
    _postorder_walk_vreg(p_head, p_des, visit_map);
}

static inline void _postorder_walk_vreg(p_list_head p_head, p_ir_vreg p_vreg, bool *visit_map) {
    assert(p_vreg);
    if (visit_map[p_vreg->id])
        return;
    visit_map[p_vreg->id] = true;

    p_list_head p_node;
    list_for_each(p_node, &p_vreg->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        switch (p_use->used_type) {
        case bb_param_ptr:
            _postorder_walk_vreg(p_head, _find_bb_phi_for_bb_param(p_use->p_bb_param)->p_bb_phi, visit_map);
            break;
        case instr_ptr:
            _postorder_walk_instr(p_head, p_use->p_instr, visit_map);
            break;
        case cond_ptr:
        case ret_ptr:
            break;
        }
    }

    p_vreg_rpo_node p_rpo_node = _vreg_rpo_node_gen(p_vreg);
    list_add_next(&p_rpo_node->node, p_head);
}

static inline void _init_rpo_list(p_list_head p_head, p_symbol_func p_func) {
    size_t vreg_cnt = p_func->param_reg_cnt + p_func->vreg_cnt;
    bool *visit_map = malloc(sizeof(bool) * vreg_cnt);
    memset(visit_map, 0, sizeof(bool) * vreg_cnt);

    p_list_head p_node;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        _postorder_walk_vreg(p_head, p_vreg, visit_map);
    }
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        p_list_head p_node;
        list_for_each(p_node, &p_bb->instr_list) {
            p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
            _postorder_walk_instr(p_head, p_instr, visit_map);
        }
    }

    for (size_t i = 0; i < vreg_cnt; ++i) {
        assert(visit_map[i]);
    }
    free(visit_map);
}

static inline void _execute_unary(p_ir_instr p_instr, hlist_hash instr_hash) {
    assert(p_instr->irkind == ir_unary);
    p_ir_unary_instr p_unary_instr = &p_instr->ir_unary;

    if (p_unary_instr->op == ir_val_assign) {
        p_list_head p_node, p_next;
        list_for_each_safe(p_node, p_next, &p_unary_instr->p_des->use_list) {
            p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
            assert(p_use->kind == reg);
            assert(p_use->p_vreg == p_unary_instr->p_des);
            switch (p_use->used_type) {
            case instr_ptr:
                ir_operand_reset_operand(p_use, p_unary_instr->p_src);
                break;
            case bb_param_ptr:
            case cond_ptr:
            case ret_ptr:
                if (p_unary_instr->p_src->kind == reg)
                    ir_operand_reset_operand(p_use, p_unary_instr->p_src);
                break;
            }
        }
    }

    // hsah & del
    p_ir_vreg p_find = _hash_find(p_instr, instr_hash);
    if (!p_find)
        return;

    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_unary_instr->p_des->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        assert(p_use->kind == reg);
        assert(p_use->p_vreg == p_unary_instr->p_des);
        ir_operand_reset_vreg(p_use, p_find);
    }
}

static inline void _execute_binary(p_ir_instr p_instr, hlist_hash instr_hash) {
    assert(p_instr->irkind == ir_binary);
    p_ir_binary_instr p_binary_instr = &p_instr->ir_binary;
    if (p_binary_instr->p_des->if_cond)
        return;

    p_ir_operand p_folded = ir_opt_const_fold(p_binary_instr->op, p_binary_instr->p_src1, p_binary_instr->p_src2);
    if (p_folded) {
        ir_instr_reset_unary(p_instr, ir_val_assign, p_folded, p_binary_instr->p_des);
        _execute_unary(p_instr, instr_hash);
        return;
    }

    p_ir_operand p_src1 = p_binary_instr->p_src1, p_src2 = p_binary_instr->p_src2;
    switch (p_binary_instr->op) {
    case ir_add_op:
        if (p_src2->kind == reg) {
            p_ir_operand p_tmp = p_src1; p_src1 = p_src2; p_src2 = p_tmp;
        }
    case ir_sub_op:
        if (p_src2->p_type->ref_level)
            break;
        if (p_src2->kind == reg)
            break;
        assert(p_src1->kind == reg);
        if ((p_src2->p_type->basic == type_i32 && p_src2->i32const == 0)
                || (p_src2->p_type->basic == type_f32 && p_src2->f32const == 0)) {
            ir_instr_reset_unary(p_instr, ir_val_assign, ir_operand_vreg_gen(p_src1->p_vreg), p_binary_instr->p_des);
            _execute_unary(p_instr, instr_hash);
            return;
        }
        break;
    case ir_mul_op:
        if (p_src2->kind == reg) {
            p_ir_operand p_tmp = p_src1; p_src1 = p_src2; p_src2 = p_tmp;
        }
    case ir_div_op:
        assert(!p_src1->p_type->ref_level);
        assert(!p_src2->p_type->ref_level);
        if (p_src2->kind == reg)
            break;
        assert(p_src1->kind == reg);
        if ((p_src2->p_type->basic == type_i32 && p_src2->i32const == 1)
                || (p_src2->p_type->basic == type_f32 && p_src2->f32const == 1)) {
            ir_instr_reset_unary(p_instr, ir_val_assign, ir_operand_vreg_gen(p_src1->p_vreg), p_binary_instr->p_des);
            _execute_unary(p_instr, instr_hash);
            return;
        }
        break;
    default:
        break;
    }

    // hsah & del
    p_ir_vreg p_find = _hash_find(p_instr, instr_hash);
    if (!p_find)
        return;

    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_binary_instr->p_des->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        assert(p_use->kind == reg);
        assert(p_use->p_vreg == p_binary_instr->p_des);
        ir_operand_reset_vreg(p_use, p_find);
    }
}

// 因为 gep 带有类型信息，不进行折叠
static inline void _execute_gep(p_ir_instr p_instr, hlist_hash instr_hash) {
    assert(p_instr->irkind == ir_gep);
    p_ir_gep_instr p_gep_instr = &p_instr->ir_gep;

    // hsah & del
    p_ir_vreg p_find = _hash_find(p_instr, instr_hash);
    if (!p_find)
        return;

    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_gep_instr->p_des->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        assert(p_use->kind == reg);
        assert(p_use->p_vreg == p_gep_instr->p_des);
        ir_operand_reset_vreg(p_use, p_find);
    }
}

// TODO need call graph
static inline void _execute_call(p_ir_instr p_instr, hlist_hash instr_hash) {}

// TODO need memery info
static inline void _execute_load(p_ir_instr p_instr, hlist_hash instr_hash) {}

// TODO need memery info
static inline void _execute_store(p_ir_instr p_instr, hlist_hash instr_hash) {}

static inline void _execute_instr(p_ir_instr p_instr, hlist_hash instr_hash) {
    switch (p_instr->irkind) {
    case ir_binary:
        _execute_binary(p_instr, instr_hash);
        break;
    case ir_unary:
        _execute_unary(p_instr, instr_hash);
        break;
    case ir_gep:
        _execute_gep(p_instr, instr_hash);
        break;
    case ir_call:
        _execute_call(p_instr, instr_hash);
        break;
    case ir_load:
        _execute_load(p_instr, instr_hash);
        break;
    case ir_store:
        _execute_store(p_instr, instr_hash);
        break;
    }
}

static inline void _execute_phi(p_ir_bb_phi p_bb_phi) {
    p_ir_vreg p_vreg = NULL;
    p_list_head p_node;
    list_for_each(p_node, &p_bb_phi->p_basic_block->prev_branch_target_list) {
        p_ir_basic_block_branch_target p_prev = list_entry(p_node, ir_branch_target_node, node)->p_target;
        assert(p_prev);
        assert(p_prev->p_block);
        assert(p_prev->p_block == p_bb_phi->p_basic_block);
        assert(p_prev->p_source_block);
        p_list_head p_node, p_node_src = p_prev->block_param.p_next;
        list_for_each(p_node, &p_bb_phi->p_basic_block->basic_block_phis) {
            assert(p_node_src != &p_prev->block_param);
            p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
            p_ir_bb_param p_param = list_entry(p_node_src, ir_bb_param, node);
            assert(p_param->p_bb_param->kind == reg);
            if (p_phi == p_bb_phi) {
                assert(p_param->p_bb_param->kind == reg);
                if (p_param->p_bb_param->p_vreg == p_bb_phi->p_bb_phi)
                    break;
                if (p_vreg && p_vreg != p_param->p_bb_param->p_vreg)
                    return;
                if (!p_vreg)
                    p_vreg = p_param->p_bb_param->p_vreg;
                break;
            }
            p_node_src = p_node_src->p_next;
        }
        assert(p_node_src != &p_prev->block_param);
    }
    assert(p_vreg); // need 1 input (no self assign) at least

    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_bb_phi->p_bb_phi->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        assert(p_use->kind == reg);
        assert(p_use->p_vreg == p_bb_phi->p_bb_phi);
        ir_operand_reset_vreg(p_use, p_vreg);
    }
}

static inline void _ir_opt_gvn_func(p_symbol_func p_func) {
    symbol_func_set_block_id(p_func);
    list_head rpo_vreg_list = list_init_head(&rpo_vreg_list);
    _init_rpo_list(&rpo_vreg_list, p_func);

    hlist_hash instr_hash = malloc(sizeof(*instr_hash) * hash_MOD);
    for (size_t i = 0; i < hash_MOD; ++i)
        hlist_head_init(instr_hash + i);

    printf("%s:\n", p_func->name);
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &rpo_vreg_list) {
        p_vreg_rpo_node p_rpo_node = list_entry(p_node, vreg_rpo_node, node);
        list_del(&p_rpo_node->node);
        p_ir_vreg p_vreg = p_rpo_node->p_vreg;
        free(p_rpo_node);

        switch (p_vreg->def_type) {
        case instr_def:
            _execute_instr(p_vreg->p_instr_def, instr_hash);
            break;
        case bb_phi_def:
            _execute_phi(p_vreg->p_bb_phi);
            break;
        case func_param_def:
            break;
        }
    }

    for (size_t i = 0; i < hash_MOD; ++i) {
        while (!hlist_head_empty(instr_hash + i)) {
            p_instr_hash_node p_in = hlist_entry((instr_hash + i)->p_first, instr_hash_node, node);
            hlist_node_del(&p_in->node);
            free(p_in);
        }
    }
    free(instr_hash);
}

void ir_opt_gvn(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        assert(p_func->p_entry_block);
        _ir_opt_gvn_func(p_func);
    }
}

