#include <hir_gen/program.h>
#include <hir_gen/symbol_table.h>

#include <hir_gen.h>
#include <symbol_gen.h>

p_hir_program hir_program_gen(void) {
    p_hir_program p_program = malloc(sizeof(*p_program));
    *p_program = (hir_program){
        .p_table = symbol_table_gen(),
        .p_store = symbol_store_gen(),
    };
    return p_program;
}
void hir_program_drop(p_hir_program p_program) {
    assert(p_program);
    symbol_table_drop(p_program->p_table);
    symbol_store_drop(p_program->p_store);
    free(p_program);
}

bool hir_symbol_sym_add(p_hir_program p_program, p_symbol_sym p_sym) {
    if (!symbol_table_sym_add(p_program->p_table, p_sym)) return false;

    if (p_sym->p_type->kind >= type_func) {
        if (p_sym->is_def) {
            symbol_store_add_def_function(p_program->p_store, p_sym);
        }
        else {
            symbol_store_add_ndef_function(p_program->p_store, p_sym);
        }
    }
    else {
        if (p_sym->is_global) {
            symbol_store_add_global(p_program->p_store, p_sym);
        }
        else {
            symbol_store_add_local(p_program->p_store, p_sym);
        }
    }

    return true;
}
p_symbol_sym hir_symbol_sym_find(p_hir_program p_program, const char *name) {
    return symbol_table_sym_find(p_program->p_table, name);
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