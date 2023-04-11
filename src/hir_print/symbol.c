#include <hir_print.h>
#include <stdio.h>

#include <hir/symbol.h>
#include <hir/type.h>

void symbol_define_print(p_symbol_sym p_sym) {
    if (!p_sym->is_def) {
        printf("\n");
        return;
    }

    if (p_sym->p_type->kind == type_func) {
        printf("\nlocal:\n");
        p_list_head p_node;
        list_for_each(p_node, &p_sym->local) {
            symbol_init_print(list_entry(p_node, symbol_sym, node));
        }
        hir_func_print(p_sym->p_func);
    }
    else {
        assert(p_sym->p_init);
        printf(" = {");
        for (size_t i = 0; i < p_sym->p_init->size; ++i) {
            if (i > 0) printf(", ");
            hir_exp_print(p_sym->p_init->memory[i]);
        }
        printf("}\n");
    }
}

void symbol_name_print(p_symbol_sym p_sym) {
    if (p_sym->p_type->kind == type_func) {
        printf("@%s", p_sym->name);
        return;
    }
    if (p_sym->is_global) {
        printf("@%s", p_sym->name);
        return;
    }
    printf("%%%s", p_sym->name);
}

void symbol_init_print(p_symbol_sym p_sym) {
    symbol_name_print(p_sym);
    printf(" -> ");
    symbol_type_print(p_sym->p_type);
    symbol_define_print(p_sym);
}

void symbol_store_print(p_symbol_store pss) {
    p_list_head p_node;
    printf("global:\n");
    list_for_each(p_node, &pss->global) {
        symbol_init_print(list_entry(p_node, symbol_sym, node));
    }
    printf("\n");

    printf("functions:\n");
    list_for_each(p_node, &pss->def_function) {
        symbol_init_print(list_entry(p_node, symbol_sym, node));
    }
    printf("\n");


    printf("extern:\n");
    list_for_each(p_node, &pss->ndef_function) {
        symbol_init_print(list_entry(p_node, symbol_sym, node));
    }
}