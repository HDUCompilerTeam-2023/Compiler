#include <symbol_gen/var.h>

#include <symbol_gen.h>

p_symbol_init symbol_init_gen(size_t size, basic_type basic) {
    p_symbol_init p_init = malloc(sizeof(*p_init));
    *p_init = (symbol_init) {
        .basic = basic,
        .size = size,
        .memory = malloc(sizeof(*p_init->memory) * size)
    };
    memset(p_init->memory, 0, sizeof(*p_init->memory) * size);
    return p_init;
}
p_symbol_init symbol_init_add(p_symbol_init p_init, size_t offset, symbol_init_val val) {
    assert(offset < p_init->size);
    p_init->memory[offset] = val;
    return p_init;
}
void symbol_init_drop(p_symbol_init p_init) {
    if (!p_init)
        return;
    free(p_init->memory);
    free(p_init);
}

p_symbol_var symbol_var_gen(const char *name, p_symbol_type p_type, bool is_const, bool is_global, p_symbol_init p_data) {
    p_symbol_var p_var = malloc(sizeof(*p_var));
    *p_var = (symbol_var) {
        .node = list_head_init(&p_var->node),
        .name = malloc(sizeof(char) * (strlen(name) + 1)),
        .p_type = p_type,
        .id = 0,
        .is_const = is_const,
        .p_init = p_data,
        .is_global = is_global,
    };
    strcpy(p_var->name, name);
    return p_var;
}
p_symbol_var symbol_temp_var_gen(p_symbol_type p_type) {
    p_symbol_var p_var = malloc(sizeof(*p_var));
    *p_var = (symbol_var) {
        .node = list_head_init(&p_var->node),
        .name = NULL,
        .p_type = p_type,
        .id = 0,
        .is_const = false,
        .p_init = NULL,
        .is_global = false,
    };
    return p_var;
}
void symbol_var_drop(p_symbol_var p_var) {
    list_del(&p_var->node);
    symbol_init_drop(p_var->p_init);
    symbol_type_drop(p_var->p_type);
    free(p_var->name);
    free(p_var);
}
