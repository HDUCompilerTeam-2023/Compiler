
#include <mir_gen.h>
#include <mir/symbol.h>
p_mir_symbol mir_declared_sym_gen(p_symbol_sym p_sym)
{
    p_mir_symbol p_mir_sym = malloc(sizeof(*p_mir_sym));
    p_mir_sym->p_type = p_sym->p_type;
    if (p_sym->is_global) 
    {
        p_mir_sym->kind = global_var;
        p_mir_sym->name = p_sym->name;
    }
    else
    {
        p_mir_sym->kind = local_var;
        p_mir_sym->id = p_sym->id;
    }
    return p_mir_sym;
}

p_mir_symbol mir_temp_sym_gen(size_t temp_id, p_symbol_type p_type)
{
    p_mir_symbol p_mir_sym = malloc(sizeof(*p_mir_sym));
    p_mir_sym->kind = temp_var;
    p_mir_sym->id = temp_id;
    p_mir_sym->p_type = p_type;

    return p_mir_sym;

}

void mir_symbol_drop(p_mir_symbol p_sym)
{
    free(p_sym);
}