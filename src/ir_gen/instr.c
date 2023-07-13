#include <ir_gen.h>
#include <ir_gen/instr.h>

static inline void _ir_instr_inner_drop(p_ir_instr p_instr) {
    switch (p_instr->irkind) {
    case ir_binary:
        ir_operand_drop(p_instr->ir_binary.p_src1);
        ir_operand_drop(p_instr->ir_binary.p_src2);
        ir_vreg_set_instr_def(p_instr->ir_binary.p_des, NULL);
        break;
    case ir_unary:
        ir_operand_drop(p_instr->ir_unary.p_src);
        ir_vreg_set_instr_def(p_instr->ir_unary.p_des, NULL);
        break;
    case ir_call:
        ir_param_list_drop(p_instr->ir_call.p_param_list);
        if (p_instr->ir_call.p_des)
            ir_vreg_set_instr_def(p_instr->ir_call.p_des, NULL);
        break;
    case ir_gep:
        ir_operand_drop(p_instr->ir_gep.p_addr);
        ir_operand_drop(p_instr->ir_gep.p_offset);
        ir_vreg_set_instr_def(p_instr->ir_gep.p_des, NULL);
        break;
    case ir_store:
        ir_operand_drop(p_instr->ir_store.p_addr);
        ir_operand_drop(p_instr->ir_store.p_src);
        break;
    case ir_load:
        ir_operand_drop(p_instr->ir_load.p_addr);
        ir_vreg_set_instr_def(p_instr->ir_load.p_des, NULL);
        break;
    }
}
void ir_instr_drop(p_ir_instr p_instr) {
    assert(p_instr);
    list_del(&p_instr->node);
    _ir_instr_inner_drop(p_instr);
    ir_bb_phi_list_drop(p_instr->p_live_in);
    ir_bb_phi_list_drop(p_instr->p_live_out);
    free(p_instr);
}

static inline void _ir_instr_init(p_ir_instr p_instr) {
    *p_instr = (ir_instr) {
        .node = list_head_init(&p_instr->node),
        .instr_id = 0,
        .p_live_in = ir_bb_phi_list_init(),
        .p_live_out = ir_bb_phi_list_init(),
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
    };
    ir_vreg_set_instr_def(p_des, p_instr);
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
    };
    ir_vreg_set_instr_def(p_des, p_instr);
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

static inline void _ir_instr_call_set(p_ir_instr p_instr, p_symbol_func p_func, p_ir_param_list p_param_list, p_ir_vreg p_des) {
    *p_instr = (ir_instr) {
        .irkind = ir_call,
        .ir_call = (ir_call_instr) {
            .p_func = p_func,
            .p_des = p_des,
            .p_param_list = p_param_list,
            .p_first_store = p_instr,
        },
        .node = p_instr->node,
        .instr_id = p_instr->instr_id,
        .p_live_in = p_instr->p_live_in,
        .p_live_out = p_instr->p_live_out,
    };
    if (p_des)
        ir_vreg_set_instr_def(p_des, p_instr);
}
void ir_instr_reset_call(p_ir_instr p_instr, p_symbol_func p_func, p_ir_param_list p_param_list, p_ir_vreg p_des) {
    _ir_instr_inner_drop(p_instr);
    _ir_instr_call_set(p_instr, p_func, p_param_list, p_des);
}
p_ir_instr ir_call_instr_gen(p_symbol_func p_func, p_ir_param_list p_param_list, p_ir_vreg p_des) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));
    _ir_instr_init(p_instr);
    _ir_instr_call_set(p_instr, p_func, p_param_list, p_des);
    return p_instr;
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
    };
    ir_vreg_set_instr_def(p_des, p_instr);
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
    };
    ir_vreg_set_instr_def(p_des, p_instr);
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
    };
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

