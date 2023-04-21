#include <hir_print.h>
#include <stdio.h>

#include <hir/program.h>
#include <hir/func.h>

#include <symbol_print.h>

int deep = 0;

void hir_program_print(p_hir_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->func) {
        p_hir_func p_func = list_entry(p_node, hir_func, node);
        hir_func_decl_print(p_func);
    }

    symbol_store_print(p_program->p_store);
}
