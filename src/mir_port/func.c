#include <mir_port/func.h>
#include <mir/func.h>
p_list_head mir_func_get_basic_block_entry(p_mir_func p_func)
{
    return &p_func->entry_block;
}

p_symbol_sym mir_func_get_sym(p_mir_func p_func)
{
    return p_func->p_func_sym;
}