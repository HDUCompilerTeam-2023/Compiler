#include <stdio.h>
#include <symbol_print.h>

#include <symbol/type.h>

void symbol_basic_type_print(basic_type b_type) {
    if (b_type == type_void)
        printf("void");
    else if (b_type == type_i32)
        printf("i32");
    else if (b_type == type_f32)
        printf("f32");
    else if (b_type == type_str)
        printf("str");
}
void symbol_type_print(p_symbol_type p_type) {
    assert(p_type);
    uint64_t arr_level = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_type->array) {
        p_symbol_type_array p_array = list_entry(p_node, symbol_type_array, node);
        printf("[%ld X ", symbol_type_array_get_size(p_array));
        ++arr_level;
    }
    assert(p_type->basic != type_void);
    symbol_basic_type_print(p_type->basic);
    for (uint64_t i = 0; i < arr_level; ++i) {
        printf("]");
    }
    for (uint64_t i = 0; i < p_type->ref_level; ++i) {
        printf("*");
    }
}
