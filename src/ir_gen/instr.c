#include <ir_gen.h>
#include <ir_gen/instr.h>

static inline void ir_operand_set_instr_use(p_ir_operand p_operand, p_ir_instr p_instr) {
    p_operand->used_type = instr_ptr;
    p_operand->p_instr = p_instr;
}
static inline void ir_vreg_set_instr_def(p_ir_vreg p_vreg, p_ir_instr p_instr) {
    p_vreg->def_type = instr_def;
    p_vreg->p_instr_def = p_instr;
}
static inline void _ir_instr_inner_drop(p_ir_instr p_instr) {
    switch (p_instr->irkind) {
    case ir_binary:
        assert(p_instr->ir_binary.p_src1->used_type == instr_ptr);
        assert(p_instr->ir_binary.p_src1->p_instr == p_instr);
        assert(p_instr->ir_binary.p_src2->used_type == instr_ptr);
        assert(p_instr->ir_binary.p_src2->p_instr == p_instr);
        ir_operand_drop(p_instr->ir_binary.p_src1);
        ir_operand_drop(p_instr->ir_binary.p_src2);
        assert(p_instr->ir_binary.p_des->def_type == instr_def);
        assert(p_instr->ir_binary.p_des->p_instr_def == p_instr);
        ir_vreg_set_instr_def(p_instr->ir_binary.p_des, NULL);
        break;
    case ir_unary:
        assert(p_instr->ir_unary.p_src->used_type == instr_ptr);
        assert(p_instr->ir_unary.p_src->p_instr == p_instr);
        ir_operand_drop(p_instr->ir_unary.p_src);
        assert(p_instr->ir_unary.p_des->def_type == instr_def);
        assert(p_instr->ir_unary.p_des->p_instr_def == p_instr);
        ir_vreg_set_instr_def(p_instr->ir_unary.p_des, NULL);
        break;
    case ir_call:
        while (!list_head_alone(&p_instr->ir_call.param_list)) {
            p_ir_param p_param = list_entry(p_instr->ir_call.param_list.p_next, ir_param, node);
            if (!p_param->is_in_mem) {
                assert(p_param->p_param->used_type == instr_ptr);
                assert(p_param->p_param->p_instr == p_instr);
                ir_operand_drop(p_param->p_param);
            }
            list_del(&p_param->node);
            free(p_param);
        }
        if (p_instr->ir_call.p_des) {
            assert(p_instr->ir_call.p_des->def_type == instr_def);
            assert(p_instr->ir_call.p_des->p_instr_def == p_instr);
            ir_vreg_set_instr_def(p_instr->ir_call.p_des, NULL);
        }
        break;
    case ir_gep:
        assert(p_instr->ir_gep.p_addr->used_type == instr_ptr);
        assert(p_instr->ir_gep.p_addr->p_instr == p_instr);
        assert(p_instr->ir_gep.p_offset->used_type == instr_ptr);
        assert(p_instr->ir_gep.p_offset->p_instr == p_instr);
        ir_operand_drop(p_instr->ir_gep.p_addr);
        ir_operand_drop(p_instr->ir_gep.p_offset);
        assert(p_instr->ir_gep.p_des->def_type == instr_def);
        assert(p_instr->ir_gep.p_des->p_instr_def == p_instr);
        ir_vreg_set_instr_def(p_instr->ir_gep.p_des, NULL);
        break;
    case ir_store:
        assert(p_instr->ir_store.p_addr->used_type == instr_ptr);
        assert(p_instr->ir_store.p_addr->p_instr == p_instr);
        assert(p_instr->ir_store.p_src->used_type == instr_ptr);
        assert(p_instr->ir_store.p_src->p_instr == p_instr);
        ir_operand_drop(p_instr->ir_store.p_addr);
        ir_operand_drop(p_instr->ir_store.p_src);
        break;
    case ir_load:
        assert(p_instr->ir_load.p_addr->used_type == instr_ptr);
        assert(p_instr->ir_load.p_addr->p_instr == p_instr);
        ir_operand_drop(p_instr->ir_load.p_addr);
        assert(p_instr->ir_load.p_des->def_type == instr_def);
        assert(p_instr->ir_load.p_des->p_instr_def == p_instr);
        ir_vreg_set_instr_def(p_instr->ir_load.p_des, NULL);
        break;
    }
}
void ir_instr_drop(p_ir_instr p_instr) {
    assert(p_instr);
    list_del(&p_instr->node);
    _ir_instr_inner_drop(p_instr);
    ir_vreg_list_drop(p_instr->p_live_in);
    ir_vreg_list_drop(p_instr->p_live_out);
    free(p_instr);
}

static inline void _ir_instr_init(p_ir_instr p_instr) {
    *p_instr = (ir_instr) {
        .node = list_head_init(&p_instr->node),
        .instr_id = 0,
        .p_live_in = ir_vreg_list_init(),
        .p_live_out = ir_vreg_list_init(),
    };
}

static inline void _ir_instr_binary_set(p_ir_instr p_instr, ir_binary_op op, p_ir_operand p_src1, p_ir_operand p_src2, p_ir_vreg p_des) {
    *p_instr = (ir_instr) {
        .irkind = ir_binary,
        .ir_binary = (ir_binary_instr) {
            .op = op,
            .p_src1 = p_src1,
            .p_src2 = p_src2,
            .p_des = p_des,
        },
        .node = p_instr->node,
        .instr_id = p_instr->instr_id,
        .p_live_in = p_instr->p_live_in,
        .p_live_out = p_instr->p_live_out,
        .p_basic_block = p_instr->p_basic_block,
    };
    ir_vreg_set_instr_def(p_des, p_instr);
    ir_operand_set_instr_use(p_src1, p_instr);
    ir_operand_set_instr_use(p_src2, p_instr);
}
void ir_instr_reset_binary(p_ir_instr p_instr, ir_binary_op op, p_ir_operand p_src1, p_ir_operand p_src2, p_ir_vreg p_des) {
    _ir_instr_inner_drop(p_instr);
    _ir_instr_binary_set(p_instr, op, p_src1, p_src2, p_des);
}
p_ir_instr ir_binary_instr_gen(ir_binary_op op, p_ir_operand p_src1, p_ir_operand p_src2, p_ir_vreg p_des) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));
    _ir_instr_init(p_instr);
    _ir_instr_binary_set(p_instr, op, p_src1, p_src2, p_des);
    return p_instr;
}

static inline void _ir_instr_unary_set(p_ir_instr p_instr, ir_unary_op op, p_ir_operand p_src, p_ir_vreg p_des) {
    *p_instr = (ir_instr) {
        .irkind = ir_unary,
        .ir_unary = (ir_unary_instr) {
            .op = op,
            .p_src = p_src,
            .p_des = p_des,
        },
        .node = p_instr->node,
        .instr_id = p_instr->instr_id,
        .p_live_in = p_instr->p_live_in,
        .p_live_out = p_instr->p_live_out,
        .p_basic_block = p_instr->p_basic_block,
    };
    ir_vreg_set_instr_def(p_des, p_instr);
    ir_operand_set_instr_use(p_src, p_instr);
}
void ir_instr_reset_unary(p_ir_instr p_instr, ir_unary_op op, p_ir_operand p_src, p_ir_vreg p_des) {
    _ir_instr_inner_drop(p_instr);
    _ir_instr_unary_set(p_instr, op, p_src, p_des);
}
p_ir_instr ir_unary_instr_gen(ir_unary_op op, p_ir_operand p_src, p_ir_vreg p_des) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));
    _ir_instr_init(p_instr);
    _ir_instr_unary_set(p_instr, op, p_src, p_des);
    return p_instr;
}

static inline void _ir_instr_call_set(p_ir_instr p_instr, p_symbol_func p_func, p_ir_vreg p_des) {
    *p_instr = (ir_instr) {
        .irkind = ir_call,
        .ir_call = (ir_call_instr) {
            .p_func = p_func,
            .p_des = p_des,
            .p_first_store = p_instr,
            .param_list = list_head_init(&p_instr->ir_call.param_list),
        },
        .node = p_instr->node,
        .instr_id = p_instr->instr_id,
        .p_live_in = p_instr->p_live_in,
        .p_live_out = p_instr->p_live_out,
        .p_basic_block = p_instr->p_basic_block,
    };
    if (p_des)
        ir_vreg_set_instr_def(p_des, p_instr);
}
void ir_instr_reset_call(p_ir_instr p_instr, p_symbol_func p_func, p_ir_vreg p_des) {
    _ir_instr_inner_drop(p_instr);
    _ir_instr_call_set(p_instr, p_func, p_des);
}
p_ir_instr ir_call_instr_gen(p_symbol_func p_func, p_ir_vreg p_des) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));
    _ir_instr_init(p_instr);
    _ir_instr_call_set(p_instr, p_func, p_des);
    return p_instr;
}

void ir_call_param_list_add(p_ir_instr p_instr, p_ir_operand p_param, bool is_stack_ptr) {
    p_ir_param p_ir_param = ir_param_gen(p_param, is_stack_ptr);
    ir_operand_set_instr_use(p_param, p_instr);
    list_add_prev(&p_ir_param->node, &p_instr->ir_call.param_list);
}

static inline void _ir_instr_gep_set(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_operand p_offset, p_ir_vreg p_des, bool is_element) {
    *p_instr = (ir_instr) {
        .irkind = ir_gep,
        .ir_gep = (ir_gep_instr) {
            .p_addr = p_addr,
            .p_des = p_des,
            .p_offset = p_offset,
            .is_element = is_element,
        },
        .node = p_instr->node,
        .instr_id = p_instr->instr_id,
        .p_live_in = p_instr->p_live_in,
        .p_live_out = p_instr->p_live_out,
        .p_basic_block = p_instr->p_basic_block,
    };
    ir_vreg_set_instr_def(p_des, p_instr);
    ir_operand_set_instr_use(p_addr, p_instr);
    ir_operand_set_instr_use(p_offset, p_instr);
}
void ir_instr_reset_gep(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_operand p_offset, p_ir_vreg p_des, bool is_element) {
    _ir_instr_inner_drop(p_instr);
    _ir_instr_gep_set(p_instr, p_addr, p_offset, p_des, is_element);
}
p_ir_instr ir_gep_instr_gen(p_ir_operand p_addr, p_ir_operand p_offset, p_ir_vreg p_des, bool is_element) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));
    _ir_instr_init(p_instr);
    _ir_instr_gep_set(p_instr, p_addr, p_offset, p_des, is_element);
    return p_instr;
}

static inline void _ir_instr_load_set(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_vreg p_des, bool is_stack_ptr) {
    *p_instr = (ir_instr) {
        .irkind = ir_load,
        .ir_load = (ir_load_instr) {
            .p_addr = p_addr,
            .p_des = p_des,
            .is_stack_ptr = is_stack_ptr,
        },
        .node = p_instr->node,
        .instr_id = p_instr->instr_id,
        .p_live_in = p_instr->p_live_in,
        .p_live_out = p_instr->p_live_out,
        .p_basic_block = p_instr->p_basic_block,
    };
    ir_vreg_set_instr_def(p_des, p_instr);
    ir_operand_set_instr_use(p_addr, p_instr);
}
void ir_instr_reset_load(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_vreg p_des, bool is_stack_ptr) {
    _ir_instr_inner_drop(p_instr);
    _ir_instr_load_set(p_instr, p_addr, p_des, is_stack_ptr);
}
p_ir_instr ir_load_instr_gen(p_ir_operand p_addr, p_ir_vreg p_des, bool is_stack_ptr) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));
    _ir_instr_init(p_instr);
    _ir_instr_load_set(p_instr, p_addr, p_des, is_stack_ptr);
    return p_instr;
}

static inline void _ir_instr_store_set(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_operand p_src, bool is_stack_ptr) {
    *p_instr = (ir_instr) {
        .irkind = ir_store,
        .ir_store = (ir_store_instr) {
            .p_addr = p_addr,
            .p_src = p_src,
            .is_stack_ptr = is_stack_ptr,
            .p_call_param = NULL,
        },
        .node = p_instr->node,
        .instr_id = p_instr->instr_id,
        .p_live_in = p_instr->p_live_in,
        .p_live_out = p_instr->p_live_out,
        .p_basic_block = p_instr->p_basic_block,
    };
    ir_operand_set_instr_use(p_addr, p_instr);
    ir_operand_set_instr_use(p_src, p_instr);
}
void ir_instr_reset_store(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_operand p_src, bool is_stack_ptr) {
    _ir_instr_inner_drop(p_instr);
    _ir_instr_store_set(p_instr, p_addr, p_src, is_stack_ptr);
}
p_ir_instr ir_store_instr_gen(p_ir_operand p_addr, p_ir_operand p_src, bool is_stack_ptr) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));
    _ir_instr_init(p_instr);
    _ir_instr_store_set(p_instr, p_addr, p_src, is_stack_ptr);
    return p_instr;
}
void ir_instr_add_prev(p_ir_instr p_prev, p_ir_instr p_next) {
    p_prev->p_basic_block = p_next->p_basic_block;
    list_add_prev(&p_prev->node, &p_next->node);
}
void ir_instr_add_next(p_ir_instr p_next, p_ir_instr p_prev) {
    p_next->p_basic_block = p_prev->p_basic_block;
    list_add_next(&p_next->node, &p_prev->node);
}

p_ir_operand ir_instr_get_src1(p_ir_instr p_instr) {
    switch (p_instr->irkind) {
    case ir_binary:
        return p_instr->ir_binary.p_src1;
    case ir_unary:
        return p_instr->ir_unary.p_src;
    case ir_load:
        return p_instr->ir_load.p_addr;
    case ir_gep:
        return p_instr->ir_gep.p_addr;
    default:
        return NULL;
    }
}
p_ir_operand ir_instr_get_src2(p_ir_instr p_instr) {
    switch (p_instr->irkind) {
    case ir_binary:
        return p_instr->ir_binary.p_src2;
    case ir_gep:
        return p_instr->ir_gep.p_offset;
    default:
        return NULL;
    }
}
p_ir_vreg ir_instr_get_des(p_ir_instr p_instr) {
    switch (p_instr->irkind) {
    case ir_binary:
        return p_instr->ir_binary.p_des;
    case ir_unary:
        return p_instr->ir_unary.p_des;
    case ir_call:
        return p_instr->ir_call.p_des;
    case ir_load:
        return p_instr->ir_load.p_des;
    case ir_gep:
        return p_instr->ir_gep.p_des;
    default:
        return NULL;
    }
}
p_ir_operand ir_instr_get_load_addr(p_ir_instr p_instr) {
    switch (p_instr->irkind) {
    case ir_load:
        return p_instr->ir_load.p_addr;
    default:
        return NULL;
    }
}
p_ir_operand ir_instr_get_store_addr(p_ir_instr p_instr) {
    switch (p_instr->irkind) {
    case ir_store:
        return p_instr->ir_store.p_addr;
    default:
        return NULL;
    }
}

void ir_set_load_instr_des(p_ir_instr p_load, p_ir_vreg p_des) {
    p_load->ir_load.p_des = p_des;
    ir_vreg_set_instr_def(p_des, p_load);
}
void ir_set_load_instr_addr(p_ir_instr p_load, p_ir_operand p_addr) {
    p_load->ir_load.p_addr = p_addr;
    ir_operand_set_instr_use(p_addr, p_load);
}
void ir_set_binary_instr_des(p_ir_instr p_binary, p_ir_vreg p_des) {
    p_binary->ir_binary.p_des = p_des;
    ir_vreg_set_instr_def(p_des, p_binary);
}
void ir_set_binary_instr_src1(p_ir_instr p_load, p_ir_operand p_src1) {
    p_load->ir_binary.p_src1 = p_src1;
    ir_operand_set_instr_use(p_src1, p_load);
}
void ir_set_binary_instr_src2(p_ir_instr p_load, p_ir_operand p_src2) {
    p_load->ir_binary.p_src2 = p_src2;
    ir_operand_set_instr_use(p_src2, p_load);
}
void ir_set_unary_instr_des(p_ir_instr p_unary, p_ir_vreg p_des) {
    p_unary->ir_unary.p_des = p_des;
    ir_vreg_set_instr_def(p_des, p_unary);
}
void ir_set_unary_instr_src(p_ir_instr p_unary, p_ir_operand p_src) {
    p_unary->ir_unary.p_src = p_src;
    ir_operand_set_instr_use(p_src, p_unary);
}
void ir_set_gep_instr_des(p_ir_instr p_gep, p_ir_vreg p_des) {
    p_gep->ir_gep.p_des = p_des;
    ir_vreg_set_instr_def(p_des, p_gep);
}
void ir_set_gep_instr_addr(p_ir_instr p_gep, p_ir_operand p_addr) {
    p_gep->ir_gep.p_addr = p_addr;
    ir_operand_set_instr_use(p_addr, p_gep);
}
void ir_set_gep_instr_offset(p_ir_instr p_gep, p_ir_operand p_offset) {
    p_gep->ir_gep.p_offset = p_offset;
    ir_operand_set_instr_use(p_offset, p_gep);
}
void ir_set_call_instr_des(p_ir_instr p_call, p_ir_vreg p_des) {
    p_call->ir_call.p_des = p_des;
    ir_vreg_set_instr_def(p_des, p_call);
}
void ir_set_store_instr_src(p_ir_instr p_store, p_ir_operand p_src) {
    p_store->ir_store.p_src = p_src;
    ir_operand_set_instr_use(p_src, p_store);
}
void ir_set_store_instr_offset(p_ir_instr p_store, p_ir_operand p_offset) {
    p_store->ir_store.p_src = p_offset;
    ir_operand_set_instr_use(p_offset, p_store);
}
