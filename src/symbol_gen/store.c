#include <symbol_gen/store.h>

#include <symbol_gen.h>

p_symbol_store symbol_store_gen(void) {
    p_symbol_store p_store = malloc(sizeof(*p_store));
    *p_store = (symbol_store) {
        .variable = list_head_init(&p_store->variable),
        .constant = list_head_init(&p_store->constant),
        .function = list_head_init(&p_store->function),
        .string = list_head_init(&p_store->string),
    };
    return p_store;
}
void symbol_store_drop(p_symbol_store p_store) {
    while (!list_head_alone(&p_store->function)) {
        p_symbol_sym p_del = list_entry(p_store->function.p_next, symbol_sym, node);
        symbol_func_drop(p_del);
    }

    while (!list_head_alone(&p_store->variable)) {
        p_symbol_sym p_del = list_entry(p_store->variable.p_next, symbol_sym, node);
        symbol_var_drop(p_del);
    }

    while (!list_head_alone(&p_store->constant)) {
        p_symbol_sym p_del = list_entry(p_store->constant.p_next, symbol_sym, node);
        symbol_var_drop(p_del);
    }

    while (!list_head_alone(&p_store->string)) {
        p_symbol_str p_del = list_entry(p_store->string.p_next, symbol_str, node);
        symbol_str_drop(p_del);
    }

    free(p_store);
}

bool symbol_store_add_str(p_symbol_store p_store, p_symbol_str p_str) {
    return list_add_prev(&p_str->node, &p_store->string);
}

bool symbol_store_add_global(p_symbol_store p_store, p_symbol_sym p_sym) {
    p_list_head p_list = &p_store->variable;
    if (p_sym->is_const) p_list = &p_store->constant;
    return list_add_prev(&p_sym->node, p_list);
}
bool symbol_store_add_local(p_symbol_store p_store, p_symbol_sym p_sym) {
    assert(!list_head_alone(&p_store->function));
    p_symbol_sym p_func = list_entry(p_store->function.p_prev, symbol_sym, node);
    p_list_head p_list = &p_func->variable;
    if (p_sym->is_const) p_list = &p_func->constant;
    return list_add_prev(&p_sym->node, p_list);
}

bool symbol_store_add_def_function(p_symbol_store p_store, p_symbol_sym p_sym) {
    return list_add_prev(&p_sym->node, &p_store->function);
}