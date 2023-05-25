#include <mir/basic_block.h>
#include <mir/func.h>
#include <mir_print.h>

#include <mir/instr.h> // 包含问题？
#include <stdio.h>

#include <symbol/var.h>
#include <symbol/func.h>
#include <symbol/type.h>

void mir_func_print(p_mir_func p_func) {
    assert(p_func);
    printf("define ");
    mir_basic_type_print(p_func->p_func_sym->ret_type);
    printf(" %s (", p_func->p_func_sym->name);
    p_list_head p_node;
    list_for_each(p_node, &p_func->p_func_sym->variable) {
        if (p_node->p_prev == p_func->p_func_sym->last_param) break;
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        mir_symbol_type_print(p_var->p_type);
        printf("%%l%ld", p_var->id);
        if (p_node != p_func->p_func_sym->last_param) printf(", ");
    }
    printf(")\n");

    if (list_head_alone(&p_func->block)) return;
    printf("{\n");
    bool is_param = true;
    list_for_each(p_node, &p_func->p_func_sym->variable) {
        if (p_node->p_prev == p_func->p_func_sym->last_param) is_param = false;
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        if (is_param)
            printf("param ");
        else
            printf("local ");
        mir_symbol_type_print(p_var->p_type);
        printf("%%l%ld\n", p_var->id);
    }

    list_for_each(p_node, &p_func->block) {
        p_mir_basic_block p_basic_block = list_entry(p_node, mir_basic_block, node);
        mir_basic_block_print(p_basic_block);
    }
    printf("}\n");
}

void mir_func_dom_info_print(p_mir_func p_func) {
    printf("function: %s\n", p_func->p_func_sym->name);
    if (list_head_alone(&p_func->block)) return;
    p_mir_basic_block p_basic_block = list_entry(p_func->block.p_next, mir_basic_block, node);
    mir_basic_block_dom_info_print(p_basic_block, 0);
}
