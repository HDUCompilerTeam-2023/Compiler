#include <symbol_gen/store.h>

#include <symbol_gen.h>
#include <hir_gen.h>
#include <mir_gen.h>

p_program symbol_store_gen(void) {
    p_program p_store = malloc(sizeof(*p_store));
    *p_store = (program) {
        .variable = list_head_init(&p_store->variable),
        .v_memory = list_head_init(&p_store->v_memory),
        .function = list_head_init(&p_store->function),
        .string = list_head_init(&p_store->string),
        .variable_cnt = 0,
        .v_memory_cnt = 0,
        .function_cnt = 0,
    };
    return p_store;
}
void symbol_store_hir_drop(p_program p_store) {
    p_list_head p_node;
    list_for_each(p_node, &p_store->function) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        if (p_sym->p_h_func)
            hir_func_drop(p_sym->p_h_func);
        p_sym->p_h_func = NULL;
    }
}
void symbol_store_mir_drop(p_program p_store) {
    p_list_head p_node, p_netx;
    list_for_each_safe(p_node, p_netx, &p_store->v_memory) {
        p_mir_vmem p_vmem = list_entry(p_node, mir_vmem, node);
        list_del(&p_vmem->node);
        mir_vmem_drop(p_vmem);
    }
    list_for_each(p_node, &p_store->function) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        if (p_sym->p_m_func)
            mir_func_drop(p_sym->p_m_func);
        p_sym->p_m_func = NULL;
    }
}
void symbol_store_drop(p_program p_store) {
    while (!list_head_alone(&p_store->function)) {
        p_symbol_sym p_del = list_entry(p_store->function.p_next, symbol_sym, node);
        assert(!p_del->p_h_func);
        assert(!p_del->p_m_func);
        symbol_func_drop(p_del);
    }

    while (!list_head_alone(&p_store->variable)) {
        p_symbol_sym p_del = list_entry(p_store->variable.p_next, symbol_sym, node);
        symbol_var_drop(p_del);
    }

    while (!list_head_alone(&p_store->string)) {
        p_symbol_str p_del = list_entry(p_store->string.p_next, symbol_str, node);
        symbol_str_drop(p_del);
    }

    free(p_store);
}

bool symbol_store_add_str(p_program p_store, p_symbol_str p_str) {
    return list_add_prev(&p_str->node, &p_store->string);
}

bool symbol_store_add_global(p_program p_store, p_symbol_sym p_sym) {
    p_sym->id = p_store->variable_cnt++;
    p_sym->is_global = true;
    return list_add_prev(&p_sym->node, &p_store->variable);
}
bool symbol_store_add_local(p_program p_store, p_symbol_sym p_sym) {
    assert(!list_head_alone(&p_store->function));
    p_symbol_sym p_func = list_entry(p_store->function.p_prev, symbol_sym, node);

    p_sym->id = p_func->variable_cnt++;
    p_sym->is_global = false;
    return list_add_prev(&p_sym->node, &p_func->variable);
}

bool symbol_store_add_function(p_program p_store, p_symbol_sym p_sym) {
    p_sym->id = p_store->function_cnt++;
    return list_add_prev(&p_sym->node, &p_store->function);
}

void symbol_store_mir_vmem_add(p_program p_store, p_mir_vmem p_vmem) {
    list_add_prev(&p_vmem->node, &p_store->v_memory);
    ++p_store->v_memory_cnt;
}
void symbol_store_mir_set_vmem_id(p_program p_store) {
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_store->v_memory) {
        p_mir_vmem p_vmem = list_entry(p_node, mir_vmem, node);
        p_vmem->id = id++;
    }
}
