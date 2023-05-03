
#include <mir_gen.h>
#include <mir_gen/operand.h>

#include <symbol/sym.h>
#include <symbol/type.h>

basic_type mir_operand_get_basic_type(p_mir_operand p_operand) {
    switch (p_operand->kind) {
    case imme:
        return p_operand->b_type;
    case mem:
        assert(p_operand->p_sym->p_type->kind != type_arrary);
        return p_operand->p_sym->p_type->basic;
    case reg:
        assert(!p_operand->p_temp_sym->is_pointer);
        return p_operand->p_temp_sym->b_type;
    }
}

p_mir_operand mir_operand_int_gen(INTCONST_t intconst) {
    p_mir_operand p_mir_int = malloc(sizeof(*p_mir_int));
    *p_mir_int = (mir_operand) {
        .intconst = intconst,
        .kind = imme,
        .b_type = type_int,
    };
    return p_mir_int;
}

p_mir_operand mir_operand_float_gen(FLOATCONST_t floatconst) {
    p_mir_operand p_mir_float = malloc(sizeof(*p_mir_float));
    *p_mir_float = (mir_operand) {
        .floatconst = floatconst,
        .kind = imme,
        .b_type = type_float,
    };
    return p_mir_float;
}

p_mir_operand mir_operand_void_gen(void) {
    p_mir_operand p_mir_void = malloc(sizeof(*p_mir_void));
    *p_mir_void = (mir_operand) {
        .kind = imme,
        .b_type = type_void,
    };
    return p_mir_void;
}
// 已定义变量转换为操作数， 全局变量或函数存储名字， 局部变量存储 id
p_mir_operand mir_operand_declared_sym_gen(p_symbol_sym p_h_sym) {
    p_mir_operand p_sym = malloc(sizeof(*p_sym));
    *p_sym = (mir_operand) {
        .kind = mem,
        .p_sym = p_h_sym,
    };
    return p_sym;
}

p_mir_operand mir_operand_copy(p_mir_operand p_operand) {
    p_mir_operand p_new_operand = malloc(sizeof(*p_new_operand));
    *p_new_operand = *p_operand;
    return p_new_operand;
}

p_mir_operand mir_operand_temp_sym_gen(p_mir_temp_sym p_temp_sym) {
    p_mir_operand p_sym = malloc(sizeof(*p_sym));
    *p_sym = (mir_operand) {
        .kind = reg,
        .p_temp_sym = p_temp_sym,
    };
    return p_sym;
}

void mir_operand_drop(p_mir_operand p_operand) {
    assert(p_operand);
    free(p_operand);
}
