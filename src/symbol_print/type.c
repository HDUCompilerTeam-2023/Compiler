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
    if (list_head_alone(&p_type->array)) {
        assert(p_type->basic != type_void);
        symbol_basic_type_print(p_type->basic);
    }
    else if (!list_head_alone(&p_type->array)) {
        p_list_head p_node;
        list_for_each(p_node, &p_type->array) {
            p_symbol_type_array p_array = list_entry(p_node, symbol_type_array, node);
            printf("arr[%ld] -> ", symbol_type_array_get_size(p_array));
        }
        assert(p_type->basic != type_void);
        symbol_basic_type_print(p_type->basic);
    }
}