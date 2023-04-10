#include <hir_print.h>
#include <stdio.h>

#include <hir/symbol.h>

void symbol_sym_print(p_symbol_sym p_sym) {
    printf("%s", p_sym->name);
}

void symbol_init_print(p_symbol_init p_init) {
    if (!p_init) return;

    printf(" = {");
    for (size_t i = 0; i < p_init->size; ++i) {
        if (i > 0) printf(", ");
        if (p_init->memory[i]) {
            hir_exp_print(p_init->memory[i]);
        }
        else {
            printf("0");
        }
    }
    printf("}");
}

void symbol_store_print(p_symbol_store pss) {
    p_symbol_sym p_node = pss->p_info;
    while (p_node) {
        symbol_type_print(p_node->p_type);
        printf(" -> ");
        symbol_sym_print(p_node);
        symbol_init_print(p_node->p_init);
        printf("\n");

        p_node = p_node->p_next;
    }
}