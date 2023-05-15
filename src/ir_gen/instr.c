#include <ir_gen.h>
#include <ir_gen/instr.h>

p_ir_instr ir_binary_instr_gen(ir_binary_op op, p_ir_operand p_src1, p_ir_operand p_src2, p_ir_vreg p_des) {
    p_ir_instr p_instr = NULL;
    p_instr = malloc(sizeof(*p_instr));
    *p_instr = (ir_instr) {
        .irkind = ir_binary,
        .ir_binary = (ir_binary_instr) {
            .op = op,
            .p_src1 = p_src1,
            .p_src2 = p_src2,
            .p_des = p_des,
        },
        .node = list_head_init(&p_instr->node),
    };
    ir_vreg_set_instr_def(p_des, p_instr);
    return p_instr;
}

p_ir_instr ir_unary_instr_gen(ir_unary_op op, p_ir_operand p_src, p_ir_vreg p_des) {
    p_ir_instr p_instr = NULL;
    p_instr = malloc(sizeof(*p_instr));
    *p_instr = (ir_instr) {
        .irkind = ir_unary,
        .ir_unary = (ir_unary_instr) {
            .op = op,
            .p_src = p_src,
            .p_des = p_des,
        },
        .node = list_head_init(&p_instr->node),
    };
    ir_vreg_set_instr_def(p_des, p_instr);
    return p_instr;
}

p_ir_instr ir_call_instr_gen(p_symbol_func p_func, p_ir_param_list p_param_list, p_ir_vreg p_des) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));

    *p_instr = (ir_instr) {
        .irkind = ir_call,
        .ir_call = (ir_call_instr) {
            .p_func = p_func,
            .p_des = p_des,
            .p_param_list = p_param_list,
        },
        .node = list_head_init(&p_instr->node),
    };
    if (p_des)
        ir_vreg_set_instr_def(p_des, p_instr);
    return p_instr;
}

p_ir_instr ir_gep_instr_gen(p_ir_operand p_addr, p_ir_operand p_offset, p_ir_vreg p_des, bool is_element) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));
    *p_instr = (ir_instr) {
        .irkind = ir_gep,
        .ir_gep = (ir_gep_instr) {
            .p_addr = p_addr,
            .p_des = p_des,
            .p_offset = p_offset,
            .is_element = is_element,
        },
        .node = list_head_init(&p_instr->node),
    };
    ir_vreg_set_instr_def(p_des, p_instr);
    return p_instr;
}

p_ir_instr ir_load_instr_gen(p_ir_operand p_addr, p_ir_operand p_offset, p_ir_vreg p_des) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));
    *p_instr = (ir_instr) {
        .irkind = ir_load,
        .ir_load = (ir_load_instr) {
            .p_addr = p_addr,
            .p_des = p_des,
            .p_offset = p_offset,
        },
        .node = list_head_init(&p_instr->node),
    };
    ir_vreg_set_instr_def(p_des, p_instr);
    return p_instr;
}

p_ir_instr ir_store_instr_gen(p_ir_operand p_addr, p_ir_operand p_offset, p_ir_operand p_src) {
    p_ir_instr p_instr = malloc(sizeof(*p_instr));
    *p_instr = (ir_instr) {
        .irkind = ir_store,
        .ir_store = (ir_store_instr) {
            .p_addr = p_addr,
            .p_src = p_src,
            .p_offset = p_offset,
        },
        .node = list_head_init(&p_instr->node),
    };
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
    case ir_load:
        return p_instr->ir_load.p_offset;
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

// 操作数已经被存在单独的列表中
void ir_instr_drop(p_ir_instr p_instr) {
    assert(p_instr);
    list_del(&p_instr->node);
    switch (p_instr->irkind) {
    case ir_binary:
        ir_operand_drop(p_instr->ir_binary.p_src1);
        ir_operand_drop(p_instr->ir_binary.p_src2);
        break;
    case ir_unary:
        ir_operand_drop(p_instr->ir_unary.p_src);
        break;
    case ir_call:
        ir_param_list_drop(p_instr->ir_call.p_param_list);
        break;
    case ir_gep:
        ir_operand_drop(p_instr->ir_gep.p_addr);
        ir_operand_drop(p_instr->ir_gep.p_offset);
        break;
    case ir_store:
        ir_operand_drop(p_instr->ir_store.p_addr);
        if (p_instr->ir_load.p_offset)
            ir_operand_drop(p_instr->ir_store.p_offset);
        ir_operand_drop(p_instr->ir_store.p_src);
        break;
    case ir_load:
        ir_operand_drop(p_instr->ir_load.p_addr);
        if (p_instr->ir_load.p_offset)
            ir_operand_drop(p_instr->ir_load.p_offset);
        break;
    }
    free(p_instr);
}
