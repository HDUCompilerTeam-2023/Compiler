#include <hir2mir.h>
#include <hir/func.h>

#include <symbol/sym.h>

// 生成函数 mir 将 p_info 传来的信息回馈给 mir_func
void hir2mir_func_gen(p_hir_func p_h_func, p_mir_func m_func_table)
{
    p_mir_func p_m_func = m_func_table + p_h_func->p_sym->id;
    p_m_func->p_func_sym = p_h_func->p_sym;

    if (!p_h_func->p_block) return;

    p_hir2mir_info p_info = hir2mir_info_gen(p_m_func, m_func_table);
    p_mir_basic_block p_entry_block = hir2mir_block_gen(p_info, p_h_func->p_block);
    mir_func_add_basic_block(p_m_func, p_entry_block);

    mir_func_temp_sym_add(p_m_func, p_info->p_ret_operand->p_temp_sym);
    hir2mir_info_add_basic_block(p_info, p_info->p_ret_block);

    mir_func_set_block_id(p_m_func);
    mir_func_set_temp_id(p_m_func);
    hir2mir_info_drop(p_info);
}