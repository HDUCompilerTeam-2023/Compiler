#include <symbol_gen/sym.h>

#include <symbol_gen.h>
#include <hir_gen.h>

p_symbol_init symbol_init_gen(size_t size) {
    p_symbol_init p_init = malloc(sizeof(*p_init));
    *p_init = (symbol_init) {
        .size = size,
        .memory = malloc(sizeof(**p_init->memory) * size)
    };
    memset(p_init->memory, 0, sizeof(**p_init->memory) * size);
    return p_init;
}
p_symbol_init symbol_init_add(p_symbol_init p_init, size_t offset, p_hir_exp p_exp) {
    assert(offset < p_init->size);
    p_init->memory[offset] = p_exp;
    return p_init;
}
void symbol_init_drop(p_symbol_init p_init) {
    if(!p_init)
        return;
    for(size_t i = 0; i < p_init->size; ++i) {
        if (p_init->memory[i]) {
            hir_exp_drop(p_init->memory[i]);
        }
    }
    free(p_init->memory);
    free(p_init);
}

static inline p_symbol_sym symbol_gen(const char *name, p_symbol_type p_type, bool is_const, bool is_def, void *p_data) {
    p_symbol_sym p_info = malloc(sizeof(*p_info));
    *p_info = (symbol_sym) {
        .node = list_head_init(&p_info->node),
        .is_def = is_def,
        .is_const = is_const,
        .name = malloc(sizeof(char) * (strlen(name) + 1)),
        .p_type = p_type,
    };
    strcpy(p_info->name, name);
    if (p_type->kind >= type_func) {
        p_info->p_func = (p_hir_func) p_data;
        p_info->local = list_head_init(&p_info->local);
    }
    else {
        p_info->p_init = (p_symbol_init) p_data;
    }
    return p_info;
}
p_symbol_sym symbol_var_gen(const char *name, p_symbol_type p_type, bool is_const, bool is_def, void *p_data) {
    return symbol_gen(name, p_type, is_const, is_def, p_data);
}
p_symbol_sym symbol_func_gen(const char *name, p_symbol_type p_type, bool is_const, bool is_def, void *p_data) {
    return symbol_gen(name, p_type, is_const, is_def, p_data);
}

void symbol_var_drop(p_symbol_sym p_sym) {
    list_del(&p_sym->node);
    symbol_init_drop(p_sym->p_init);
    symbol_type_drop(p_sym->p_type);
    free(p_sym->name);
    free(p_sym);
}

void symbol_func_drop(p_symbol_sym p_sym) {
    list_del(&p_sym->node);
    if (!p_sym->is_def) {
        symbol_type_drop_param(p_sym->p_type);
    }
    else {
        while (!list_head_alone(&p_sym->local)) {
            p_symbol_sym p_del = list_entry(p_sym->local.p_next, symbol_sym, node);
            symbol_var_drop(p_del);
        }
        hir_func_drop(p_sym->p_func);
    }
    symbol_type_drop(p_sym->p_type);
    free(p_sym->name);
    free(p_sym);
}