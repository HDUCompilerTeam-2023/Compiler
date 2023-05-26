#include <stdio.h>
#include <symbol_print.h>

#include <symbol/type.h>

void symbol_basic_type_print(basic_type b_type) {
    if (b_type == type_void)
        printf("void");
    else if (b_type == type_int)
        printf("int");
    else if (b_type == type_float)
        printf("float");
    else if (b_type == type_str)
        printf("string");
}
void symbol_type_print(p_symbol_type p_type) {
    assert(p_type);
    if (p_type->kind == type_var) {
        assert(p_type->basic != type_void);
        symbol_basic_type_print(p_type->basic);
    }
    else if (p_type->kind == type_arrary) {
        printf("arr[%ld] -> ", p_type->size);
        symbol_type_print(p_type->p_item);
    }
}