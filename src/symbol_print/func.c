#include "symbol.h"
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
    p_list_head p_node;
    printf(" (");
    list_for_each(p_node, &p_func->param) {
        p_symbol_var p_param = list_entry(p_node, symbol_var, node);
        printf("param: ");
        symbol_type_print(p_param->p_type);
        if (p_node->p_next != &p_func->param) {
            printf(", ");
        }
    }
    if (p_func->is_va) {
        if (!list_head_alone(&p_func->param)) printf(", ");
        printf("param: ...");
    }
    printf(")\n");
    if (!list_head_alone(&p_func->param)) {
        printf("param:\n");
    }
    list_for_each(p_node, &p_func->param) {
        symbol_init_print(list_entry(p_node, symbol_var, node));
    }
}
