#include <mir_gen/func.h>
#include <mir_gen.h>

p_mir_func mir_func_gen(p_symbol_sym p_func_sym){
    p_mir_func p_func = malloc(sizeof(*p_func));
    *p_func = (mir_func){
        .p_func_sym = p_func_sym,
        .p_basic_block = NULL,
        .node = list_head_init(&p_func->node),
        .p_operand_list = NULL,
        .temp_sym_head = list_head_init(&p_func->temp_sym_head),
    };
    return p_func;
}

p_mir_func mir_func_set_block(p_mir_func p_func, p_mir_basic_block p_block)
{
    p_func->p_basic_block = p_block;
    return p_func;
}

void mir_func_temp_sym_add(p_mir_func p_func, p_mir_temp_sym p_temp_sym)
{
    list_add_prev(&p_temp_sym->node, &p_func->temp_sym_head);
}

void mir_func_drop(p_mir_func p_func)
{
    assert(p_func);
    while (p_func->p_basic_block) {
        p_mir_basic_block p_del = p_func->p_basic_block;
        p_func->p_basic_block = p_del->p_next;
        mir_basic_block_drop(p_del);
    }
    while(!list_head_alone(&p_func->temp_sym_head))
    {
        p_mir_temp_sym p_temp_sym = list_entry(p_func->temp_sym_head.p_next, mir_temp_sym, node);
        list_del(&p_temp_sym->node);
        free(p_temp_sym);
    }
    mir_operand_list_drop(p_func->p_operand_list);
    free(p_func);
}