#include <stdio.h>
#include <symbol_print.h>
#include <hir_print.h>
#include <mir_print.h>

#include <symbol/store.h>
#include <symbol/str.h>
#include <symbol/sym.h>

void symbol_store_print(p_program p_store) {
    p_list_head p_node;
    if (!list_head_alone(&p_store->variable)) {
        printf("global:\n");
    }
    list_for_each(p_node, &p_store->variable) {
        symbol_init_print(list_entry(p_node, symbol_sym, node));
    }

    if (!list_head_alone(&p_store->function)) {
        if (!list_head_alone(&p_store->variable))
            printf("\n");
        printf("functions:\n");
    }
    list_for_each(p_node, &p_store->function) {
        symbol_init_print(list_entry(p_node, symbol_sym, node));
    }

    if (!list_head_alone(&p_store->string)) {
        if (!list_head_alone(&p_store->function))
            printf("\n");
        printf("string:\n");
    }
    list_for_each(p_node, &p_store->string) {
        printf("%s\n", list_entry(p_node, symbol_str, node)->string);
    }
}

void symbol_store_hir_print(p_program p_store) {
    p_list_head p_node;
    list_for_each(p_node, &p_store->function) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        if (p_sym->p_h_func)
            hir_func_decl_print(p_sym->p_h_func);
    }
}

void symbol_store_mir_print(p_program p_store) {
    assert(p_store);
    printf("=== mir program start ===\n");
    p_list_head p_node;
    list_for_each(p_node, &p_store->variable) {
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

    list_for_each(p_node, &p_store->function) {
        p_mir_func p_func = list_entry(p_node, symbol_sym, node)->p_m_func;
        mir_func_print(p_func);
    }
    printf(" === mir program end ===\n");
}
void symbol_store_mir_dom_info_print(p_program p_store) {
    printf("+++ dom_tree start +++\n");
    p_list_head p_node;
    list_for_each(p_node, &p_store->function) {
        p_mir_func p_func = list_entry(p_node, symbol_sym, node)->p_m_func;
        mir_func_dom_info_print(p_func);
    }
    printf("+++ dom_tree end +++\n");
}
