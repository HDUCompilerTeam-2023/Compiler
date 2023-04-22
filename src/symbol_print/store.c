#include <symbol_print.h>
#include <stdio.h>

#include <symbol/store.h>
#include <symbol/str.h>
#include <symbol/sym.h>

void symbol_store_print(p_symbol_store p_store) {
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