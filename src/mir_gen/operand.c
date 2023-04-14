
#include <mir_gen.h>
#include <mir_gen/operand.h>


p_mir_operand mir_operand_int_gen(int intconst)
{
    p_mir_operand p_mir_int = malloc(sizeof(*p_mir_int));
    *p_mir_int = (mir_operand){
        .intconst = intconst,
        .kind = int_val,
    };
    return p_mir_int;
}

p_mir_operand mir_operand_float_gen(float floatconst)
{
    p_mir_operand p_mir_float = malloc(sizeof(*p_mir_float));
    *p_mir_float = (mir_operand){
        .floatconst = floatconst,
        .kind = float_val,
    };
    return p_mir_float;
}

p_mir_operand p_mir_operand_void_gen(void)
{
    p_mir_operand p_mir_float = malloc(sizeof(*p_mir_float));
    *p_mir_float = (mir_operand){
        .kind = void_val,
    };
    return p_mir_float;
}

p_mir_operand mir_operand_sym_gen(p_mir_symbol p_mir_sym)
{
    p_mir_operand p_sym = malloc(sizeof(*p_sym));
    *p_sym = (mir_operand){
        .p_sym = p_mir_sym,
        .kind = sym,
    };
    return p_sym;
}



void mir_operand_drop(p_mir_operand p_operand)
{
    assert(p_operand);
    if (p_operand->kind == sym) {
        free(p_operand->p_sym);
    }
    free(p_operand);
}