#include <symbol_print.h>
#include <stdio.h>

#include <symbol/type.h>

void symbol_type_print(p_symbol_type p_type) {
    assert(p_type);
    if (p_type->kind == type_var) {
        assert(p_type->basic != type_void);

        if (p_type->basic == type_int) printf("int");
        else if (p_type->basic == type_float) printf("float");
        else if (p_type->basic == type_str) printf("string");
    }
    else if (p_type->kind >= type_func) {
        if (p_type->basic == type_void) printf("void ");
        else if (p_type->basic == type_int) printf("int ");
        else if (p_type->basic == type_float) printf("float ");
        printf("(");
        if (p_type->p_params) symbol_type_print(p_type->p_params);
        if (p_type->kind == type_va_func) {
            if (p_type->p_params) printf(", ");
            printf("param: ...");
        }
        printf(")");
    }
    else if (p_type->kind == type_param) {
        printf("param: ");

        symbol_type_print(p_type->p_item);
        if (p_type->p_params) {
            printf(", ");
            symbol_type_print(p_type->p_params);
        }
    }
    else if (p_type->kind == type_arrary) {
        printf("arr[%ld] -> ", p_type->size);
        symbol_type_print(p_type->p_item);
    }
}