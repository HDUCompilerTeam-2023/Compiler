#include <hir_print.h>
#include <stdio.h>

#include <hir/program.h>
#include <hir/func.h>

int deep = 0;

void hir_program_print(p_hir_program p_program) {
    assert(p_program);
    printf("functions:\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->func) {
        p_hir_func p_func = list_entry(p_node, hir_func, node);
        hir_func_print(p_func);
    }
    printf("\n");
    printf("symbols:\n");
    symbol_store_print(p_program->pss);
}
