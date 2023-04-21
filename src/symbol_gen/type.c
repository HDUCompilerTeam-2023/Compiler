#include <symbol_gen/type.h>

p_symbol_type symbol_type_var_gen(basic_type basic) {
    p_symbol_type p_type = malloc(sizeof(*p_type));
    *p_type = (symbol_type) {
        .kind = type_var,
        .basic = basic,
        .size = 1,
    };
    return p_type;
}
p_symbol_type symbol_type_arrary_gen(size_t size) {
    p_symbol_type p_type = malloc(sizeof(*p_type));
    *p_type = (symbol_type) {
        .kind = type_arrary,
        .p_item = NULL,
        .size = size,
    };
    return p_type;
}
p_symbol_type symbol_type_func_gen(bool is_va) {
    p_symbol_type p_type = malloc(sizeof(*p_type));
    *p_type = (symbol_type) {
        .kind = is_va ? type_va_func : type_func,
        .basic = type_void,
        .p_params = NULL,
    };
    return p_type;
}
p_symbol_type symbol_type_param_gen(p_symbol_type p_param) {
    p_symbol_type p_type = malloc(sizeof(*p_type));
    *p_type = (symbol_type) {
        .kind = type_param,
        .p_item = p_param,
        .p_params = NULL,
    };
    return p_type;
}

void symbol_type_drop(p_symbol_type p_type) {
    assert(p_type);
    if (p_type->kind == type_var) {
        assert(p_type->basic != type_void);
    }
    else if (p_type->kind >= type_func) {
        if (p_type->p_params) symbol_type_drop(p_type->p_params);
    }
    else if (p_type->kind == type_param) {
        if (p_type->p_params) symbol_type_drop(p_type->p_params);
    }
    else if (p_type->kind == type_arrary) {
        symbol_type_drop(p_type->p_item);
    }
    free(p_type);
}