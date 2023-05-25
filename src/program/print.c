#include <hir_print.h>
#include <mir_print.h>
#include <stdio.h>
#include <symbol_print.h>

#include <program/def.h>
#include <program/use.h>
#include <symbol/func.h>
#include <symbol/str.h>
#include <symbol/var.h>

void program_variable_print(p_program p_program) {
    p_list_head p_node;
    if (!list_head_alone(&p_program->variable)) {
        printf("global:\n");
    }
    list_for_each(p_node, &p_program->variable) {
        symbol_init_print(list_entry(p_node, symbol_var, node));
    }

    if (!list_head_alone(&p_program->string)) {
        if (!list_head_alone(&p_program->variable))
            printf("\n");
        printf("string:\n");
    }
    list_for_each(p_node, &p_program->string) {
        printf("%s\n", list_entry(p_node, symbol_str, node)->string);
    }
}

void program_hir_print(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (!p_func->p_h_block) continue;
        symbol_func_init_print(p_func);
        printf("{\n");
        deep += 4;
        hir_block_print(p_func->p_h_block);
        deep -= 4;
        printf("}\n");
    }
}

void program_mir_print(p_program p_program) {
    assert(p_program);
    printf("=== mir program start ===\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        printf("global ");
        mir_symbol_type_print(p_var->p_type);
        printf("%s ", p_var->name);
        if (p_var->is_def) {
            printf("= {");
            for (size_t i = 0; i < p_var->p_init->size; i++) {
                if (i > 0) printf(", ");
                if (p_var->p_init->basic == type_int)
                    printf("%ld", p_var->p_init->memory[i].i);
                else
                    printf("%lf", p_var->p_init->memory[i].f);
            }
            printf("}");
        }
        printf("\n");
    }

    list_for_each(p_node, &p_program->function) {
        p_mir_func p_func = list_entry(p_node, symbol_func, node)->p_m_func;
        mir_func_print(p_func);
    }
    printf(" === mir program end ===\n");
}
void program_mir_dom_info_print(p_program p_program) {
    printf("+++ dom_tree start +++\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_mir_func p_func = list_entry(p_node, symbol_func, node)->p_m_func;
        mir_func_dom_info_print(p_func);
    }
    printf("+++ dom_tree end +++\n");
}
