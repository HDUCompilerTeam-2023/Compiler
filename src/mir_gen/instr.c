#include <mir_gen.h>
#include <mir_gen/instr.h>

p_mir_instr mir_binary_instr_gen(mir_binary_op op, p_mir_operand p_src1, p_mir_operand p_src2, p_mir_vreg p_des) {
    p_mir_instr p_instr = NULL;
    p_instr = malloc(sizeof(*p_instr));
    *p_instr = (mir_instr) {
        .irkind = mir_binary,
        .mir_binary = (mir_binary_instr) {
            .op = op,
            .p_src1 = p_src1,
            .p_src2 = p_src2,
            .p_des = p_des,
        },
        .node = list_head_init(&p_instr->node),
    };
    mir_vreg_set_instr_def(p_des, p_instr);
    return p_instr;
}

p_mir_instr mir_unary_instr_gen(mir_unary_op op, p_mir_operand p_src, p_mir_vreg p_des) {
    p_mir_instr p_instr = NULL;
    p_instr = malloc(sizeof(*p_instr));
    *p_instr = (mir_instr) {
        .irkind = mir_unary,
        .mir_unary = (mir_unary_instr) {
            .op = op,
            .p_src = p_src,
            .p_des = p_des,
        },
        .node = list_head_init(&p_instr->node),
    };
    mir_vreg_set_instr_def(p_des, p_instr);
    return p_instr;
}

p_mir_instr mir_call_instr_gen(p_symbol_func p_func, p_mir_param_list p_param_list, p_mir_vreg p_des) {
    p_mir_instr p_instr = malloc(sizeof(*p_instr));

    *p_instr = (mir_instr) {
        .irkind = mir_call,
        .mir_call = (mir_call_instr) {
            .p_func = p_func,
            .p_des = p_des,
            .p_param_list = p_param_list,
        },
        .node = list_head_init(&p_instr->node),
    };
    mir_vreg_set_instr_def(p_des, p_instr);
    return p_instr;
}

p_mir_instr mir_load_instr_gen(p_mir_operand p_addr, p_mir_operand p_offset, p_mir_vreg p_des) {
    p_mir_instr p_instr = malloc(sizeof(*p_instr));
    *p_instr = (mir_instr) {
        .irkind = mir_load,
        .mir_load = (mir_load_instr) {
            .p_addr = p_addr,
            .p_des = p_des,
            .p_offset = p_offset,
        },
        .node = list_head_init(&p_instr->node),
    };
    mir_vreg_set_instr_def(p_des, p_instr);
    return p_instr;
}

p_mir_instr mir_store_instr_gen(p_mir_operand p_addr, p_mir_operand p_offset, p_mir_operand p_src) {
    p_mir_instr p_instr = malloc(sizeof(*p_instr));
    *p_instr = (mir_instr) {
        .irkind = mir_store,
        .mir_store = (mir_store_instr) {
            .p_addr = p_addr,
            .p_src = p_src,
            .p_offset = p_offset,
        },
        .node = list_head_init(&p_instr->node),
    };
    return p_instr;
}

p_mir_operand mir_instr_get_src1(p_mir_instr p_instr) {
    switch (p_instr->irkind) {
    case mir_binary:
        return p_instr->mir_binary.p_src1;
    case mir_unary:
        return p_instr->mir_unary.p_src;
    default:
        return NULL;
    }
}
p_mir_operand mir_instr_get_src2(p_mir_instr p_instr) {
    switch (p_instr->irkind) {
    case mir_binary:
        return p_instr->mir_binary.p_src2;
    default:
        return NULL;
    }
}
p_mir_vreg mir_instr_get_des(p_mir_instr p_instr) {
    switch (p_instr->irkind) {
    case mir_binary:
        return p_instr->mir_binary.p_des;
    case mir_unary:
        return p_instr->mir_unary.p_des;
    case mir_call:
        return p_instr->mir_call.p_des;
    case mir_load:
        return p_instr->mir_load.p_des;
    default:
        return NULL;
    }
}
p_mir_operand mir_instr_get_load_addr(p_mir_instr p_instr) {
    switch (p_instr->irkind) {
    case mir_load:
        return p_instr->mir_load.p_addr;
    default:
        return NULL;
    }
}
p_mir_operand mir_instr_get_store_addr(p_mir_instr p_instr) {
    switch (p_instr->irkind) {
    case mir_store:
        return p_instr->mir_store.p_addr;
    default:
        return NULL;
    }
}

// 操作数已经被存在单独的列表中
void mir_instr_drop(p_mir_instr p_instr) {
    assert(p_instr);
    list_del(&p_instr->node);
    switch (p_instr->irkind) {
    case mir_binary:
        mir_operand_drop(p_instr->mir_binary.p_src1);
        mir_operand_drop(p_instr->mir_binary.p_src2);
        break;
    case mir_unary:
        mir_operand_drop(p_instr->mir_unary.p_src);
        break;
    case mir_call:
        mir_param_list_drop(p_instr->mir_call.p_param_list);
        break;
    case mir_store:
        mir_operand_drop(p_instr->mir_store.p_addr);
        if (p_instr->mir_load.p_offset)
            mir_operand_drop(p_instr->mir_store.p_offset);
        mir_operand_drop(p_instr->mir_store.p_src);
        break;
    case mir_load:
        mir_operand_drop(p_instr->mir_load.p_addr);
        if (p_instr->mir_load.p_offset)
            mir_operand_drop(p_instr->mir_load.p_offset);
        break;
    }
    free(p_instr);
}
