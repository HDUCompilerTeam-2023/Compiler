#include <mir_gen/func.h>
#include <mir_gen.h>

p_mir_func mir_func_gen(p_symbol_sym p_func_sym, p_mir_basic_block p_block)
{
    p_mir_func p_func = malloc(sizeof(*p_func_sym));
    *p_func = (mir_func){
        .p_func_sym = p_func_sym,
        .p_basic_block = p_block,
        .node = list_head_init(&p_func->node),
        .temp_id = 0,
    };
    return p_func;
}

void mir_func_drop(p_mir_func p_func)
{
    assert(p_func);
    mir_basic_block_drop(p_func->p_basic_block);
    free(p_func);
}