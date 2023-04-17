#include <hir2mir.h>
#include <hir/func.h>

p_mir_func hir2mir_func_gen(p_hir2mir_info p_info, p_symbol_sym p_func_sym)
{
    p_info->p_basic_block_list = mir_basic_block_list_gen();
    p_info->id = 0;
    p_mir_func p_m_func = mir_func_gen(p_func_sym);
    p_mir_basic_block p_block = hir2mir_block_gen(p_info, p_func_sym->p_func->p_block);
    mir_func_set_block(p_m_func, p_block);
    p_m_func->p_basic_block_list = p_info->p_basic_block_list;
    return p_m_func;
}