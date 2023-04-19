#include <hir2mir.h>
#include <hir/func.h>
// 生成函数 mir 将 p_info 传来的信息回馈给 mir_func
p_mir_func hir2mir_func_gen(p_symbol_sym p_func_sym)
{
    p_mir_func p_m_func = mir_func_gen(p_func_sym);
    p_hir2mir_info p_info = hir2mir_info_gen(p_func_sym);
    p_mir_basic_block p_block = hir2mir_block_gen(p_info, p_func_sym->p_func->p_block);
    mir_func_set_block(p_m_func, p_block);
    p_m_func->p_basic_block_list = p_info->p_basic_block_list;
    p_m_func->p_operand_list = p_info->p_operand_list;;
    mir_basic_block_set_id(p_info->id, p_block);
    hir2mir_info_drop(p_info);
    return p_m_func;
}