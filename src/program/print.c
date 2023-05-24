#include <stdio.h>
#include <symbol_print.h>
#include <hir_print.h>
#include <mir_print.h>

#include <program/use.h>
#include <program/def.h>
#include <symbol/str.h>
#include <symbol/sym.h>

void program_variable_print(p_program p_program) {
    p_list_head p_node;
    if (!list_head_alone(&p_program->variable)) {
        printf("global:\n");
    }
    list_for_each(p_node, &p_program->variable) {
        symbol_init_print(list_entry(p_node, symbol_sym, node));
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
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        if (p_sym->p_h_func)
            hir_func_decl_print(p_sym->p_h_func);
    }
}

void program_mir_print(p_program p_program) {
    assert(p_program);
    printf("=== mir program start ===\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->variable) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        printf("global ");
        mir_symbol_type_print(p_sym->p_type);
        printf("%s ", p_sym->name);
        if (p_sym->is_def) {
            printf("= {");
            for (size_t i = 0; i < p_sym->p_init->size; i++) {
                if (i > 0) printf(", ");
                if (p_sym->p_init->basic == type_int)
                    printf("%ld", p_sym->p_init->memory[i].i);
                else
                    printf("%lf", p_sym->p_init->memory[i].f);
            }
            printf("}");
        }
        printf("\n");
    }

    list_for_each(p_node, &p_program->function) {
        p_mir_func p_func = list_entry(p_node, symbol_sym, node)->p_m_func;
        mir_func_print(p_func);
    }
    printf(" === mir program end ===\n");
}
void program_mir_dom_info_print(p_program p_program) {
    printf("+++ dom_tree start +++\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_mir_func p_func = list_entry(p_node, symbol_sym, node)->p_m_func;
        mir_func_dom_info_print(p_func);
    }
    printf("+++ dom_tree end +++\n");
}
