
#include <mir_gen.h>
#include <mir_gen/operand.h>

p_mir_symbol mir_declared_sym_gen(p_symbol_sym p_sym)
{
    p_mir_symbol p_mir_sym = malloc(sizeof(*p_mir_sym));
    p_mir_sym->p_type = p_sym->p_type;
    if (p_sym->is_global) 
    {
        p_mir_sym->irsym_kind = global_var;
        p_mir_sym->name = p_sym->name;
    }
    else
    {
        p_mir_sym->irsym_kind = local_var;
        p_mir_sym->id = p_sym->id;
    }
    return p_mir_sym;
}

p_mir_symbol mir_temp_sym_gen(p_mir_func p_func, p_symbol_type p_type)
{
    p_mir_symbol p_mir_sym = malloc(sizeof(*p_mir_sym));
    p_mir_sym->irsym_kind = temp_var;
    p_mir_sym->id = p_func->temp_id;
    p_mir_sym->p_type = p_type;

    p_func->temp_id += 1;
    return p_mir_sym;

}
p_mir_operand mir_operand_int_gen(int intconst)
{
    p_mir_operand p_mir_int = malloc(sizeof(*p_mir_int));
    *p_mir_int = (mir_operand){
        .intconst = intconst,
        .irop_kind = int_val,
    };
    return p_mir_int;
}

p_mir_operand mir_operand_float_gen(float floatconst)
{
    p_mir_operand p_mir_float = malloc(sizeof(*p_mir_float));
    *p_mir_float = (mir_operand){
        .floatconst = floatconst,
        .irop_kind = float_val,
    };
    return p_mir_float;
}

p_mir_operand mir_operand_sym_gen(p_mir_symbol p_mir_sym)
{
    p_mir_operand p_sym = malloc(sizeof(*p_sym));
    *p_sym = (mir_operand){
        .p_sym = p_mir_sym,
        .irop_kind = sym,
    };
    return p_sym;
}

void mir_symbol_drop(p_mir_symbol p_sym)
{
    free(p_sym);
}

void mir_operand_drop(p_mir_operand p_operand)
{
    assert(p_operand);
    if (p_operand->irop_kind == sym) {
        free(p_operand->p_sym);
    }
    free(p_operand);
}