#include <ir/instr.h>
#include <ir_gen/operand.h>
#include <symbol/type.h>

static inline p_ir_operand _ir_opt_const_fold_add(p_ir_operand p_src1, p_ir_operand p_src2) {
    if (p_src1->kind == reg || p_src2->kind == reg)
        return NULL;
    assert(p_src1->kind == imme && p_src2->kind == imme);

    if (p_src2->p_type->ref_level) {
        p_ir_operand p_tmp = p_src1;
        p_src1 = p_src2;
        p_src2 = p_tmp;
    }
    assert(!p_src2->p_type->ref_level);

    if (p_src1->p_type->ref_level) {
        assert(p_src1->p_type->ref_level == 1);
        assert(p_src2->p_type->basic == type_i32);
        size_t offset = p_src1->offset + p_src2->i32const;
        return ir_operand_addr_gen(p_src1->p_vmem, p_src1->p_type, offset);
    }

    assert(p_src1->p_type->basic == p_src2->p_type->basic);
    if (p_src1->p_type->basic == type_i32) {
        return ir_operand_int_gen(p_src1->i32const + p_src2->i32const);
    }
    assert(p_src1->p_type->basic == type_f32);
    return ir_operand_float_gen(p_src1->f32const + p_src2->f32const);
}

static inline p_ir_operand _ir_opt_const_fold_sub(p_ir_operand p_src1, p_ir_operand p_src2) {
    if (p_src1->kind == reg && p_src2->kind == reg && p_src1->p_vreg == p_src2->p_vreg) {
        if (p_src1->p_type->basic == type_i32)
            return ir_operand_int_gen(0);
        assert(p_src1->p_type->basic == type_f32);
        return ir_operand_float_gen(0);
    }
    if (p_src1->kind == reg || p_src2->kind == reg)
        return NULL;
    assert(p_src1->kind == imme && p_src2->kind == imme);

    assert(!p_src2->p_type->ref_level);

    if (p_src1->p_type->ref_level) {
        assert(p_src1->p_type->ref_level == 1);
        assert(p_src2->p_type->basic == type_i32);
        size_t offset = p_src1->offset - p_src2->i32const;
        return ir_operand_addr_gen(p_src1->p_vmem, p_src1->p_type, offset);
    }
    assert(p_src1->p_type->basic == p_src2->p_type->basic);
    if (p_src1->p_type->basic == type_i32) {
        return ir_operand_int_gen(p_src1->i32const - p_src2->i32const);
    }
    assert(p_src1->p_type->basic == type_f32);
    return ir_operand_float_gen(p_src1->f32const - p_src2->f32const);
}

static inline p_ir_operand _ir_opt_const_fold_mul(p_ir_operand p_src1, p_ir_operand p_src2) {
    assert(!p_src1->p_type->ref_level);
    assert(!p_src2->p_type->ref_level);
    assert(p_src1->p_type->basic == p_src2->p_type->basic);

    if (p_src1->kind == reg && p_src2->kind == reg)
        return NULL;
    if (p_src2->kind == reg) {
        p_ir_operand p_tmp = p_src1; p_src1 = p_src2; p_src2 = p_tmp;
    }
    if (p_src1->kind == reg) {
        if (p_src2->p_type->basic == type_i32 && p_src2->i32const == 0)
            return ir_operand_int_gen(0);
        if (p_src2->p_type->basic == type_f32 && p_src2->f32const == 0)
            return ir_operand_float_gen(0);
        return NULL;
    }
    if (p_src1->p_type->basic == type_i32)
        return ir_operand_int_gen(p_src1->i32const * p_src2->i32const);
    assert(p_src1->p_type->basic == type_f32);
    return ir_operand_float_gen(p_src1->f32const * p_src2->f32const);
}

static inline p_ir_operand _ir_opt_const_fold_div(p_ir_operand p_src1, p_ir_operand p_src2) {
    assert(!p_src1->p_type->ref_level);
    assert(!p_src2->p_type->ref_level);
    assert(p_src1->p_type->basic == p_src2->p_type->basic);

    if (p_src1->kind == reg && p_src2->kind == reg && p_src1->p_vreg == p_src2->p_vreg) {
        if (p_src1->p_type->basic == type_i32)
            return ir_operand_int_gen(1);
        assert(p_src1->p_type->basic == type_f32);
        return ir_operand_float_gen(1);
    }
    if (p_src1->kind == imme) {
        if (p_src1->p_type->basic == type_i32 && p_src1->i32const == 0)
            return ir_operand_int_gen(0);
        if (p_src1->p_type->basic == type_f32 && p_src1->f32const == 0)
            return ir_operand_float_gen(0);
    }
    if (p_src1->kind == reg || p_src2->kind == reg)
        return NULL;
    if (p_src1->p_type->basic == type_i32)
        return ir_operand_int_gen(p_src1->i32const / p_src2->i32const);
    assert(p_src1->p_type->basic == type_f32);
    return ir_operand_float_gen(p_src1->f32const / p_src2->f32const);
}

static inline p_ir_operand _ir_opt_const_fold_mod(p_ir_operand p_src1, p_ir_operand p_src2) {
    assert(!p_src1->p_type->ref_level);
    assert(!p_src2->p_type->ref_level);
    assert(p_src1->p_type->basic == type_i32);
    assert(p_src2->p_type->basic == type_i32);

    if (p_src1->kind == reg && p_src2->kind == reg && p_src1->p_vreg == p_src2->p_vreg)
        return ir_operand_int_gen(0);
    if (p_src1->kind == imme && p_src1->i32const == 0)
        return ir_operand_int_gen(0);
    if (p_src2->kind == imme && (p_src2->i32const == 1 || p_src2->i32const == -1))
        return ir_operand_int_gen(0);
    if (p_src1->kind == reg || p_src2->kind == reg)
        return NULL;
    return ir_operand_int_gen(p_src1->i32const % p_src2->i32const);
}

static inline p_ir_operand _ir_opt_const_fold_g(p_ir_operand p_src1, p_ir_operand p_src2) {
    assert(!p_src1->p_type->ref_level);
    assert(!p_src2->p_type->ref_level);
    assert(p_src1->p_type->basic == p_src2->p_type->basic);

    if (p_src1->kind == reg && p_src2->kind == reg && p_src1->p_vreg == p_src2->p_vreg)
        return ir_operand_int_gen(0);
    if (p_src1->kind == reg || p_src2->kind == reg)
        return NULL;
    if (p_src1->p_type->basic == type_i32)
        return ir_operand_int_gen(p_src1->i32const > p_src2->i32const);
    assert(p_src1->p_type->basic == type_f32);
    return ir_operand_int_gen(p_src1->f32const > p_src2->f32const);
}

static inline p_ir_operand _ir_opt_const_fold_geq(p_ir_operand p_src1, p_ir_operand p_src2) {
    assert(!p_src1->p_type->ref_level);
    assert(!p_src2->p_type->ref_level);
    assert(p_src1->p_type->basic == p_src2->p_type->basic);

    if (p_src1->kind == reg && p_src2->kind == reg && p_src1->p_vreg == p_src2->p_vreg)
        return ir_operand_int_gen(1);
    if (p_src1->kind == reg || p_src2->kind == reg)
        return NULL;
    if (p_src1->p_type->basic == type_i32)
        return ir_operand_int_gen(p_src1->i32const >= p_src2->i32const);
    assert(p_src1->p_type->basic == type_f32);
    return ir_operand_int_gen(p_src1->f32const >= p_src2->f32const);
}

static inline p_ir_operand _ir_opt_const_fold_l(p_ir_operand p_src1, p_ir_operand p_src2) {
    assert(!p_src1->p_type->ref_level);
    assert(!p_src2->p_type->ref_level);
    assert(p_src1->p_type->basic == p_src2->p_type->basic);

    if (p_src1->kind == reg && p_src2->kind == reg && p_src1->p_vreg == p_src2->p_vreg)
        return ir_operand_int_gen(0);
    if (p_src1->kind == reg || p_src2->kind == reg)
        return NULL;
    if (p_src1->p_type->basic == type_i32)
        return ir_operand_int_gen(p_src1->i32const < p_src2->i32const);
    assert(p_src1->p_type->basic == type_f32);
    return ir_operand_int_gen(p_src1->f32const < p_src2->f32const);
}

static inline p_ir_operand _ir_opt_const_fold_leq(p_ir_operand p_src1, p_ir_operand p_src2) {
    assert(!p_src1->p_type->ref_level);
    assert(!p_src2->p_type->ref_level);
    assert(p_src1->p_type->basic == p_src2->p_type->basic);

    if (p_src1->kind == reg && p_src2->kind == reg && p_src1->p_vreg == p_src2->p_vreg)
        return ir_operand_int_gen(1);
    if (p_src1->kind == reg || p_src2->kind == reg)
        return NULL;
    if (p_src1->p_type->basic == type_i32)
        return ir_operand_int_gen(p_src1->i32const <= p_src2->i32const);
    assert(p_src1->p_type->basic == type_f32);
    return ir_operand_int_gen(p_src1->f32const <= p_src2->f32const);
}

static inline p_ir_operand _ir_opt_const_fold_eq(p_ir_operand p_src1, p_ir_operand p_src2) {
    assert(!p_src1->p_type->ref_level);
    assert(!p_src2->p_type->ref_level);
    assert(p_src1->p_type->basic == p_src2->p_type->basic);

    if (p_src1->kind == reg && p_src2->kind == reg && p_src1->p_vreg == p_src2->p_vreg)
        return ir_operand_int_gen(1);
    if (p_src1->kind == reg || p_src2->kind == reg)
        return NULL;
    if (p_src1->p_type->basic == type_i32)
        return ir_operand_int_gen(p_src1->i32const == p_src2->i32const);
    assert(p_src1->p_type->basic == type_f32);
    return ir_operand_int_gen(p_src1->f32const == p_src2->f32const);
}

static inline p_ir_operand _ir_opt_const_fold_neq(p_ir_operand p_src1, p_ir_operand p_src2) {
    assert(!p_src1->p_type->ref_level);
    assert(!p_src2->p_type->ref_level);
    assert(p_src1->p_type->basic == p_src2->p_type->basic);

    if (p_src1->kind == reg && p_src2->kind == reg && p_src1->p_vreg == p_src2->p_vreg)
        return ir_operand_int_gen(0);
    if (p_src1->kind == reg || p_src2->kind == reg)
        return NULL;
    if (p_src1->p_type->basic == type_i32)
        return ir_operand_int_gen(p_src1->i32const != p_src2->i32const);
    assert(p_src1->p_type->basic == type_f32);
    return ir_operand_int_gen(p_src1->f32const != p_src2->f32const);
}

p_ir_operand ir_opt_const_fold(ir_binary_op b_op, p_ir_operand p_src1, p_ir_operand p_src2) {
    assert(list_head_alone(&p_src1->p_type->array));
    assert(list_head_alone(&p_src2->p_type->array));

    p_ir_operand p_ret = NULL;
    switch (b_op) {
    case ir_add_op:
        p_ret = _ir_opt_const_fold_add(p_src1, p_src2);
        break;
    case ir_sub_op:
        p_ret = _ir_opt_const_fold_sub(p_src1, p_src2);
        break;
    case ir_mul_op:
        p_ret = _ir_opt_const_fold_mul(p_src1, p_src2);
        break;
    case ir_div_op:
        p_ret = _ir_opt_const_fold_div(p_src1, p_src2);
        break;
    case ir_mod_op:
        p_ret = _ir_opt_const_fold_mod(p_src1, p_src2);
        break;
    case ir_g_op:
        p_ret = _ir_opt_const_fold_g(p_src1, p_src2);
        break;
    case ir_geq_op:
        p_ret = _ir_opt_const_fold_geq(p_src1, p_src2);
        break;
    case ir_l_op:
        p_ret = _ir_opt_const_fold_l(p_src1, p_src2);
        break;
    case ir_leq_op:
        p_ret = _ir_opt_const_fold_leq(p_src1, p_src2);
        break;
    case ir_eq_op:
        p_ret = _ir_opt_const_fold_eq(p_src1, p_src2);
        break;
    case ir_neq_op:
        p_ret = _ir_opt_const_fold_neq(p_src1, p_src2);
        break;
    }
    return p_ret;
}
