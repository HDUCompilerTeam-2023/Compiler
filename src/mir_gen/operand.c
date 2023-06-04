
#include <mir_gen.h>
#include <mir_gen/operand.h>

#include <symbol/var.h>
#include <symbol/type.h>
#include <symbol_gen/type.h>

basic_type mir_operand_get_basic_type(p_mir_operand p_operand) {
    return p_operand->p_type->basic;
}

p_mir_operand mir_operand_str_gen(p_symbol_str strconst) {
    p_mir_operand p_mir_int = malloc(sizeof(*p_mir_int));
    *p_mir_int = (mir_operand) {
        .strconst = strconst,
        .kind = imme,
        .p_type = symbol_type_var_gen(type_str),
    };
    return p_mir_int;
}

p_mir_operand mir_operand_int_gen(INTCONST_t intconst) {
    p_mir_operand p_mir_int = malloc(sizeof(*p_mir_int));
    *p_mir_int = (mir_operand) {
        .intconst = intconst,
        .kind = imme,
        .p_type = symbol_type_var_gen(type_int),
    };
    return p_mir_int;
}

p_mir_operand mir_operand_float_gen(FLOATCONST_t floatconst) {
    p_mir_operand p_mir_float = malloc(sizeof(*p_mir_float));
    *p_mir_float = (mir_operand) {
        .floatconst = floatconst,
        .kind = imme,
        .p_type = symbol_type_var_gen(type_float),
    };
    return p_mir_float;
}

p_mir_operand mir_operand_void_gen(void) {
    p_mir_operand p_mir_void = malloc(sizeof(*p_mir_void));
    *p_mir_void = (mir_operand) {
        .kind = imme,
        .p_type = symbol_type_var_gen(type_void),
    };
    return p_mir_void;
}

p_mir_operand mir_operand_addr_gen(p_mir_vmem p_global_vmem) {
    p_mir_operand p_operand = malloc(sizeof(*p_operand));
    *p_operand = (mir_operand) {
        .kind = imme,
        .p_global_vmem = p_global_vmem,
        .p_type = symbol_type_copy(p_global_vmem->p_type),
    };
    if (!list_head_alone(&p_operand->p_type->array) && p_operand->p_type->ref_level == 0) {
        symbol_type_array_drop(symbol_type_pop_array(p_operand->p_type));
    }
    symbol_type_push_ptr(p_operand->p_type);
    return p_operand;
}

p_mir_operand mir_operand_vreg_gen(p_mir_vreg p_vreg) {
    p_mir_operand p_operand = malloc(sizeof(*p_operand));
    *p_operand = (mir_operand) {
        .kind = reg,
        .p_vreg = p_vreg,
        .use_node = list_head_init(&p_operand->use_node),
        .p_type = symbol_type_copy(p_vreg->p_type),
    };
    return p_operand;
}

void mir_operand_drop(p_mir_operand p_operand) {
    assert(p_operand);
    symbol_type_drop(p_operand->p_type);
    free(p_operand);
}
