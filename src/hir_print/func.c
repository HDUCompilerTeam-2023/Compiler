#include <hir_print.h>
#include <stdio.h>

#include <hir/func.h>
#include <symbol_print.h>

void hir_func_call_print(p_hir_func p_func, p_hir_param_list p_param_list) {
    assert(p_func);
    symbol_name_print(p_func->p_sym);
    printf("(");
    hir_param_list_print(p_param_list);
    printf(")");
}

void hir_func_decl_print(p_hir_func p_func) {
    assert(p_func);
    if (p_func->p_block) {
        symbol_init_print(p_func->p_sym);
        symbol_param_print(p_func->p_sym);
        hir_block_print(p_func->p_block);
    }
}
