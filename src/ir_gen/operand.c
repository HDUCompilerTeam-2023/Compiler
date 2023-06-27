
#include <ir_gen.h>
#include <ir_gen/operand.h>

#include <symbol/type.h>
#include <symbol/var.h>
#include <symbol_gen/type.h>

basic_type ir_operand_get_basic_type(p_ir_operand p_operand) {
    return p_operand->p_type->basic;
}

p_ir_operand ir_operand_str_gen(p_symbol_str strconst) {
    p_ir_operand p_ir_int = malloc(sizeof(*p_ir_int));
    *p_ir_int = (ir_operand) {
        .strconst = strconst,
        .kind = imme,
        .p_type = symbol_type_var_gen(type_str),
    };
    return p_ir_int;
}

p_ir_operand ir_operand_int_gen(I32CONST_t intconst) {
    p_ir_operand p_ir_int = malloc(sizeof(*p_ir_int));
    *p_ir_int = (ir_operand) {
        .i32const = intconst,
        .kind = imme,
        .p_type = symbol_type_var_gen(type_i32),
    };
    return p_ir_int;
}

p_ir_operand ir_operand_float_gen(F32CONST_t floatconst) {
    p_ir_operand p_ir_float = malloc(sizeof(*p_ir_float));
    *p_ir_float = (ir_operand) {
        .f32const = floatconst,
        .kind = imme,
        .p_type = symbol_type_var_gen(type_f32),
    };
    return p_ir_float;
}

p_ir_operand ir_operand_void_gen(void) {
    p_ir_operand p_ir_void = malloc(sizeof(*p_ir_void));
    *p_ir_void = (ir_operand) {
        .kind = imme,
        .p_type = symbol_type_var_gen(type_void),
    };
    return p_ir_void;
}

p_ir_operand ir_operand_addr_gen(p_symbol_var p_vmem) {
    p_ir_operand p_operand = malloc(sizeof(*p_operand));
    *p_operand = (ir_operand) {
        .kind = imme,
        .p_vmem = p_vmem,
        .p_type = symbol_type_copy(p_vmem->p_type),
    };
    symbol_type_push_ptr(p_operand->p_type);
    return p_operand;
}

p_ir_operand ir_operand_vreg_gen(p_ir_vreg p_vreg) {
    p_ir_operand p_operand = malloc(sizeof(*p_operand));
    *p_operand = (ir_operand) {
        .kind = reg,
        .p_vreg = p_vreg,
        .use_node = list_head_init(&p_operand->use_node),
        .p_type = symbol_type_copy(p_vreg->p_type),
    };
    return p_operand;
}

void ir_operand_drop(p_ir_operand p_operand) {
    assert(p_operand);
    symbol_type_drop(p_operand->p_type);
    free(p_operand);
}
