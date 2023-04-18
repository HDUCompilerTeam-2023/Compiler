#include <mir_gen/func.h>
#include <mir_gen.h>

p_mir_func mir_func_gen(p_symbol_sym p_func_sym){
    p_mir_func p_func = malloc(sizeof(*p_func));
    *p_func = (mir_func){
        .p_func_sym = p_func_sym,
        .p_basic_block = NULL,
        .node = list_head_init(&p_func->node),
        .p_basic_block_list = NULL,
        .p_operand_list = NULL,
    };
    return p_func;
}

p_mir_func mir_func_set_block(p_mir_func p_func, p_mir_basic_block p_block)
{
    p_func->p_basic_block = p_block;
    return p_func;
}

void mir_func_drop(p_mir_func p_func)
{
    assert(p_func);
    mir_basic_block_list_drop(p_func->p_basic_block_list);
    mir_operand_list_drop(p_func->p_operand_list);
    free(p_func);
}