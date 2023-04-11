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
        p_symbol_sym p_node = p_sym->p_local;
        printf("\nlocal:\n");
        while (p_node) {
            symbol_init_print(p_node);
            p_node = p_node->p_next;
        }
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
    if (p_sym->is_global) {
        printf("@%s", p_sym->name);
    }
    else {
        printf("%%%s", p_sym->name);
    }
}

void symbol_init_print(p_symbol_sym p_sym) {
    symbol_name_print(p_sym);
    printf(" -> ");
    symbol_type_print(p_sym->p_type);
    symbol_define_print(p_sym);
}

void symbol_store_print(p_symbol_store pss) {
    p_symbol_sym p_node;
    printf("global:\n");
    p_node = pss->p_global;
    while (p_node) {
        symbol_init_print(p_node);

        p_node = p_node->p_next;
    }
    printf("\n");

    p_node = pss->p_def_function;
    printf("functions:\n");
    while (p_node) {
        symbol_init_print(p_node);

        hir_func_print(p_node->p_func);

        p_node = p_node->p_next;
    }
    printf("\n");

    p_node = pss->p_ndef_function;
    printf("extern:\n");
    while (p_node) {
        symbol_init_print(p_node);

        p_node = p_node->p_next;
    }
}