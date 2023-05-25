#include <stdio.h>
#include <symbol_print.h>

#include <hir_print.h>
#include <symbol/func.h>
#include <symbol/type.h>
#include <symbol/var.h>

void symbol_func_name_print(p_symbol_func p_func) {
    printf("@%s", p_func->name);
}

void symbol_func_init_print(p_symbol_func p_func) {
    symbol_func_name_print(p_func);
    printf(" -> ");
    assert(p_func->ret_type != type_str);

    symbol_basic_type_print(p_func->ret_type);
    printf(" (");
    if (p_func->p_params) symbol_type_print(p_func->p_params);
    if (p_func->is_va) {
        if (p_func->p_params) printf(", ");
        printf("param: ...");
    }
    printf(")\n");
}

void symbol_func_param_print(p_symbol_func p_func) {
    p_list_head p_node;
    if (p_func->last_param != &p_func->variable) {
        printf("param:\n");
    }
    list_for_each(p_node, &p_func->variable) {
        if (p_node->p_prev == p_func->last_param)
            break;
        symbol_init_print(list_entry(p_node, symbol_var, node));
    }
}
