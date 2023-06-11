#include <stdio.h>
#include <symbol_print.h>

#include <symbol/func.h>
#include <symbol/type.h>
#include <symbol/var.h>

void symbol_func_name_print(p_symbol_func p_func) {
    printf("%s", p_func->name);
}

void symbol_func_init_print(p_symbol_func p_func) {
    printf("define ");
    symbol_basic_type_print(p_func->ret_type);
    printf(" ");
    symbol_func_name_print(p_func);
    printf(" (");
    p_list_head p_node;
    list_for_each(p_node, &p_func->param) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        symbol_name_print(p_var);
        if (p_node != p_func->param.p_prev) printf(", ");
    }
    printf(")\n");
}
