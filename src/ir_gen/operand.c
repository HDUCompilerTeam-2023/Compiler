#include <ir_gen.h>
#include <ir_gen/operand.h>

#include <symbol/type.h>
#include <symbol/var.h>
#include <symbol_gen/type.h>

basic_type ir_operand_get_basic_type(p_ir_operand p_operand) {
    return p_operand->p_type->basic;
}

static inline void _ir_operand_inner_drop(p_ir_operand p_operand) {
    if (p_operand->kind == reg)
        assert(list_del(&p_operand->use_node));
    symbol_type_drop(p_operand->p_type);
}
void ir_operand_drop(p_ir_operand p_operand) {
    assert(p_operand);
    _ir_operand_inner_drop(p_operand);
    free(p_operand);
}

static inline void _ir_operand_str_set(p_ir_operand p_operand, p_symbol_str strconst) {
    *p_operand = (ir_operand) {
        .strconst = strconst,
        .kind = imme,
        .p_type = symbol_type_var_gen(type_str),
        .used_type = p_operand->used_type,
        .p_bb_param = p_operand->p_bb_param,
    };
}
void ir_operand_reset_str(p_ir_operand p_operand, p_symbol_str strconst) {
    assert(p_operand);
    if (p_operand->kind == imme && p_operand->p_type->ref_level == 0 && p_operand->p_type->basic == type_str && p_operand->strconst == strconst)
        return;
    _ir_operand_inner_drop(p_operand);
    _ir_operand_str_set(p_operand, strconst);
}
p_ir_operand ir_operand_str_gen(p_symbol_str strconst) {
    p_ir_operand p_ir_str = malloc(sizeof(*p_ir_str));
    _ir_operand_str_set(p_ir_str, strconst);
    return p_ir_str;
}

static inline void _ir_operand_int_set(p_ir_operand p_operand, I32CONST_t intconst) {
    *p_operand = (ir_operand) {
        .i32const = intconst,
        .kind = imme,
        .p_type = symbol_type_var_gen(type_i32),
        .used_type = p_operand->used_type,
        .p_bb_param = p_operand->p_bb_param,
    };
}
void ir_operand_reset_int(p_ir_operand p_operand, I32CONST_t intconst) {
    assert(p_operand);
    if (p_operand->kind == imme && p_operand->p_type->ref_level == 0 && p_operand->p_type->basic == type_i32 && p_operand->i32const == intconst)
        return;
    _ir_operand_inner_drop(p_operand);
    _ir_operand_int_set(p_operand, intconst);
}
p_ir_operand ir_operand_int_gen(I32CONST_t intconst) {
    p_ir_operand p_ir_int = malloc(sizeof(*p_ir_int));
    _ir_operand_int_set(p_ir_int, intconst);
    return p_ir_int;
}

static inline void _ir_operand_float_set(p_ir_operand p_operand, F32CONST_t floatconst) {
    *p_operand = (ir_operand) {
        .f32const = floatconst,
        .kind = imme,
        .p_type = symbol_type_var_gen(type_f32),
        .used_type = p_operand->used_type,
        .p_bb_param = p_operand->p_bb_param,
    };
}
void ir_operand_reset_float(p_ir_operand p_operand, F32CONST_t floatconst) {
    assert(p_operand);
    if (p_operand->kind == imme && p_operand->p_type->ref_level == 0 && p_operand->p_type->basic == type_f32 && p_operand->f32const == floatconst)
        return;
    _ir_operand_inner_drop(p_operand);
    _ir_operand_float_set(p_operand, floatconst);
}
p_ir_operand ir_operand_float_gen(F32CONST_t floatconst) {
    p_ir_operand p_ir_float = malloc(sizeof(*p_ir_float));
    _ir_operand_float_set(p_ir_float, floatconst);
    return p_ir_float;
}


static inline void _ir_operand_void_set(p_ir_operand p_operand) {
    *p_operand = (ir_operand) {
        .kind = imme,
        .p_type = symbol_type_var_gen(type_void),
        .used_type = p_operand->used_type,
        .p_bb_param = p_operand->p_bb_param,
    };
}
void ir_operand_reset_void(p_ir_operand p_operand) {
    assert(p_operand);
    if (p_operand->kind == imme && p_operand->p_type->ref_level == 0 && p_operand->p_type->basic == type_void)
        return;
    _ir_operand_inner_drop(p_operand);
    _ir_operand_void_set(p_operand);
}
p_ir_operand ir_operand_void_gen(void) {
    p_ir_operand p_ir_void = malloc(sizeof(*p_ir_void));
    _ir_operand_void_set(p_ir_void);
    return p_ir_void;
}

static inline void _ir_operand_addr_set(p_ir_operand p_operand, p_symbol_var p_vmem) {
    *p_operand = (ir_operand) {
        .kind = imme,
        .p_vmem = p_vmem,
        .p_type = symbol_type_copy(p_vmem->p_type),
        .used_type = p_operand->used_type,
        .p_bb_param = p_operand->p_bb_param,
    };
    symbol_type_push_ptr(p_operand->p_type);
}
p_ir_operand ir_operand_addr_gen(p_symbol_var p_vmem) {
    p_ir_operand p_operand = malloc(sizeof(*p_operand));
    _ir_operand_addr_set(p_operand, p_vmem);
    return p_operand;
}
void ir_operand_reset_addr(p_ir_operand p_operand, p_symbol_var p_vmem) {
    assert(p_operand);
    if (p_operand->kind == imme && p_operand->p_type->ref_level > 0 && p_operand->p_vmem == p_vmem)
        return;
    _ir_operand_inner_drop(p_operand);
    _ir_operand_addr_set(p_operand, p_vmem);
}

static inline void _ir_operand_vreg_set(p_ir_operand p_operand, p_ir_vreg p_vreg) {
    *p_operand = (ir_operand) {
        .kind = reg,
        .p_vreg = p_vreg,
        .use_node = list_head_init(&p_operand->use_node),
        .p_type = symbol_type_copy(p_vreg->p_type),
        .used_type = p_operand->used_type,
        .p_bb_param = p_operand->p_bb_param,
    };
    assert(list_add_prev(&p_operand->use_node, &p_vreg->use_list));
}
p_ir_operand ir_operand_vreg_gen(p_ir_vreg p_vreg) {
    p_ir_operand p_operand = malloc(sizeof(*p_operand));
    _ir_operand_vreg_set(p_operand, p_vreg);
    return p_operand;
}
void ir_operand_reset_vreg(p_ir_operand p_operand, p_ir_vreg p_vreg) {
    assert(p_operand);
    if (p_operand->kind == reg && p_operand->p_vreg == p_vreg)
        return;
    _ir_operand_inner_drop(p_operand);
    _ir_operand_vreg_set(p_operand, p_vreg);
}

p_ir_operand ir_operand_copy(p_ir_operand p_src) {
    p_ir_operand p_ret;
    if (p_src->kind == reg){
        p_ret = ir_operand_vreg_gen(p_src->p_vreg);
        p_ret->used_type = p_src->used_type;
        p_ret->p_bb_param = p_src->p_bb_param;
        return p_ret;
    }
    if (p_src->p_type->ref_level > 0){
        p_ret = ir_operand_addr_gen(p_src->p_vmem);
        p_ret->used_type = p_src->used_type;
        p_ret->p_bb_param = p_src->p_bb_param;
        return p_ret;
    }
    if (p_src->p_type->basic == type_i32)
        p_ret = ir_operand_int_gen(p_src->i32const);
    if (p_src->p_type->basic == type_f32)
        p_ret = ir_operand_float_gen(p_src->f32const);
    if (p_src->p_type->basic == type_str)
        p_ret = ir_operand_str_gen(p_src->strconst);
    if (p_src->p_type->basic == type_void)
        p_ret = ir_operand_void_gen();
    p_ret->used_type = p_src->used_type;
    p_ret->p_bb_param = p_src->p_bb_param;
    return p_ret;
}
void ir_operand_reset_operand(p_ir_operand p_operand, p_ir_operand p_src) {
    if (p_src->kind == reg) {
        ir_operand_reset_vreg(p_operand, p_src->p_vreg);
        return;
    }
    if (p_src->p_type->ref_level > 0) {
        ir_operand_reset_addr(p_operand, p_src->p_vmem);
        return;
    }
    assert(list_head_alone(&p_src->p_type->array));
    if (p_src->p_type->basic == type_i32) {
        ir_operand_reset_int(p_operand, p_src->i32const);
        return;
    }
    if (p_src->p_type->basic == type_f32) {
        ir_operand_reset_float(p_operand, p_src->f32const);
        return;
    }
    if (p_src->p_type->basic == type_str) {
        ir_operand_reset_str(p_operand, p_src->strconst);
        return;
    }
    ir_operand_reset_void(p_operand);
}
