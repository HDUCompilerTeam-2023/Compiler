#include <hir_print.h>
#include <stdio.h>

#include <hir/symbol.h>
#include <hir/type.h>

void symbol_sym_print(p_symbol_sym p_sym) {
    printf("@%s", p_sym->name);
}

void symbol_init_print(p_symbol_init p_init) {
    if (!p_init) return;

    printf(" = {");
    for (size_t i = 0; i < p_init->size; ++i) {
        if (i > 0) printf(", ");
        hir_exp_print(p_init->memory[i]);
    }
    printf("}");
}

void symbol_store_print(p_symbol_store pss) {
    p_symbol_sym p_node;
    printf("global:\n");
    p_node = pss->p_global;
    while (p_node) {
        symbol_sym_print(p_node);
        printf(" -> ");
        symbol_type_print(p_node->p_type);
        symbol_init_print(p_node->p_init);
        printf("\n");

        p_node = p_node->p_next;
    }
    printf("\n");

    p_node = pss->p_def_function;
    printf("functions:\n");
    while (p_node) {
        symbol_sym_print(p_node);
        printf(" -> ");
        symbol_type_print(p_node->p_type);
        printf("\n");

        p_symbol_sym p_node_l = p_node->p_local;
        printf("local:\n");
        while (p_node_l) {
            symbol_sym_print(p_node_l);
            printf(" -> ");
            symbol_type_print(p_node_l->p_type);
            symbol_init_print(p_node_l->p_init);
            printf("\n");

            p_node_l = p_node_l->p_next;
        }

        hir_func_print(p_node->p_func);

        p_node = p_node->p_next;
    }
    printf("\n");

    p_node = pss->p_ndef_function;
    printf("extern:\n");
    while (p_node) {
        symbol_sym_print(p_node);
        printf(" -> ");
        symbol_type_print(p_node->p_type);
        printf("\n");

        p_node = p_node->p_next;
    }
}