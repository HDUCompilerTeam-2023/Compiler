#include <hir_gen/program.h>
#include <hir_gen/symbol_table.h>

#include <hir_gen.h>
#include <symbol_gen.h>

p_hir_program hir_program_gen(void) {
    p_hir_program p_program = malloc(sizeof(*p_program));
    *p_program = (hir_program){
        .p_table = symbol_table_gen(),
        .p_store = symbol_store_gen(),
        .func = list_init_head(&p_program->func),
    };
    return p_program;
}
void hir_program_drop(p_hir_program p_program) {
    assert(p_program);
    symbol_table_drop(p_program->p_table);

    while (!list_head_alone(&p_program->func)) {
        p_hir_func p_del = list_entry(p_program->func.p_next, hir_func, node);
        hir_func_drop(p_del);
    }

    free(p_program);
}

p_hir_program hir_program_func_add(p_hir_program p_program, p_hir_func p_func) {
    list_add_prev(&p_func->node, &p_program->func);
    return p_program;
}

p_symbol_item hir_symbol_item_add(p_hir_program p_program, p_symbol_sym p_sym, bool is_global) {
    p_symbol_item p_item = symbol_table_item_add(p_program->p_table, p_sym);
    if (!p_item) return NULL;

    if (p_sym->p_type->kind >= type_func) {
        symbol_store_add_function(p_program->p_store, p_sym);
    }
    else {
        if (is_global) {
            symbol_store_add_global(p_program->p_store, p_sym);
        }
        else {
            symbol_store_add_local(p_program->p_store, p_sym);
        }
    }

    return p_item;
}
p_symbol_item hir_symbol_item_find(p_hir_program p_program, const char *name) {
    return symbol_table_item_find(p_program->p_table, name);
}

p_symbol_str hir_symbol_str_get(p_hir_program p_program, const char *string) {
    p_symbol_str p_str = symbol_table_str_get(p_program->p_table, string);
    symbol_store_add_str(p_program->p_store, p_str);
    return p_str;
}

void hir_symbol_zone_push(p_hir_program p_program) {
    symbol_table_zone_push(p_program->p_table);
}
void hir_symbol_zone_pop(p_hir_program p_program) {
    symbol_table_zone_pop(p_program->p_table);
}