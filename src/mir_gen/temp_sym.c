#include <mir_gen.h>
#include <mir/temp_sym.h>
p_mir_temp_sym mir_temp_sym_basic_gen(basic_type b_type, p_mir_func p_func)
{
    p_mir_temp_sym p_temp_sym = malloc(sizeof(*p_temp_sym));
    *p_temp_sym = (mir_temp_sym){
        .b_type = b_type,
        .is_pointer = false,
        .node = list_head_init(&p_temp_sym->node),
    };
    mir_func_temp_sym_add(p_func, p_temp_sym);
    return p_temp_sym;
}

p_mir_temp_sym mir_temp_sym_pointer_gen(basic_type b_type, p_mir_func p_func)
{
    p_mir_temp_sym p_temp_sym = malloc(sizeof(*p_temp_sym));
    *p_temp_sym = (mir_temp_sym){
        .b_type = b_type,
        .is_pointer = true,
        .node = list_head_init(&p_temp_sym->node),
    };
    mir_func_temp_sym_add(p_func, p_temp_sym);
    return p_temp_sym;
}

void mir_temp_sym_drop(p_mir_temp_sym p_temp_sym)
{
    free(p_temp_sym);
}