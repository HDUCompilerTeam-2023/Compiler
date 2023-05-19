
#include <mir_gen.h>
#include <mir_gen/operand.h>

#include <symbol/sym.h>
#include <symbol/type.h>

basic_type mir_operand_get_basic_type(p_mir_operand p_operand) {
    switch (p_operand->kind) {
    case imme:
        return p_operand->b_type;
    case reg:
        assert(p_operand->p_vreg->ref_level == 0);
        return p_operand->p_vreg->b_type;
    }
}

p_mir_operand mir_operand_str_gen(p_symbol_str strconst) {
    p_mir_operand p_mir_int = malloc(sizeof(*p_mir_int));
    *p_mir_int = (mir_operand) {
        .strconst = strconst,
        .kind = imme,
        .b_type = type_str,
        .ref_level = 0,
    };
    return p_mir_int;
}

p_mir_operand mir_operand_int_gen(INTCONST_t intconst) {
    p_mir_operand p_mir_int = malloc(sizeof(*p_mir_int));
    *p_mir_int = (mir_operand) {
        .intconst = intconst,
        .kind = imme,
        .b_type = type_int,
        .ref_level = 0,
    };
    return p_mir_int;
}

p_mir_operand mir_operand_float_gen(FLOATCONST_t floatconst) {
    p_mir_operand p_mir_float = malloc(sizeof(*p_mir_float));
    *p_mir_float = (mir_operand) {
        .floatconst = floatconst,
        .kind = imme,
        .b_type = type_float,
        .ref_level = 0,
    };
    return p_mir_float;
}

p_mir_operand mir_operand_void_gen(void) {
    p_mir_operand p_mir_void = malloc(sizeof(*p_mir_void));
    *p_mir_void = (mir_operand) {
        .kind = imme,
        .b_type = type_void,
        .ref_level = 0,
    };
    return p_mir_void;
}

p_mir_operand mir_operand_addr_gen(p_mir_vmem p_global_vmem) {
    p_mir_operand p_operand = malloc(sizeof(*p_operand));
    *p_operand = (mir_operand) {
        .kind = imme,
        .p_global_vmem = p_global_vmem,
        .b_type = p_global_vmem->b_type,
        .ref_level = p_global_vmem->ref_level + 1,
    };
    return p_operand;
}

p_mir_operand mir_operand_vreg_gen(p_mir_vreg p_vreg) {
    p_mir_operand p_operand = malloc(sizeof(*p_operand));
    *p_operand = (mir_operand) {
        .kind = reg,
        .p_vreg = p_vreg,
        .use_node = list_head_init(&p_operand->use_node),
        .b_type = p_vreg->b_type,
        .ref_level = p_vreg->ref_level,
    };
    return p_operand;
}

void mir_operand_drop(p_mir_operand p_operand) {
    assert(p_operand);
    free(p_operand);
}
