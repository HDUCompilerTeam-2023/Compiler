#include <hir_print.h>
#include <stdio.h>

#include <hir/symbol_store.h>
#include <hir/symbol.h>

void symbol_store_print(p_symbol_store pss) {
    p_list_head p_node;
    if (!list_head_alone(&pss->global)) {
        printf("global:\n");
    }
    list_for_each(p_node, &pss->global) {
        symbol_init_print(list_entry(p_node, symbol_sym, node));
    }

    if (!list_head_alone(&pss->def_function)) {
        if (!list_head_alone(&pss->global))
            printf("\n");
        printf("functions:\n");
    }
    list_for_each(p_node, &pss->def_function) {
        symbol_init_print(list_entry(p_node, symbol_sym, node));
    }

    if (!list_head_alone(&pss->ndef_function)) {
        if (!list_head_alone(&pss->def_function))
            printf("\n");
        printf("extern:\n");
    }
    list_for_each(p_node, &pss->ndef_function) {
        symbol_init_print(list_entry(p_node, symbol_sym, node));
    }

    if (!list_head_alone(&pss->string)) {
        if (!list_head_alone(&pss->ndef_function))
            printf("\n");
        printf("string:\n");
    }
    list_for_each(p_node, &pss->string) {
        printf("%s\n", list_entry(p_node, symbol_str, node)->string);
    }
}