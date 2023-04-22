#include <hir2mir.h>
#include <hir/func.h>

#include <symbol/sym.h>

// 生成函数 mir 将 p_info 传来的信息回馈给 mir_func
p_mir_func hir2mir_func_gen(p_hir_func p_h_func)
{
    p_mir_func p_m_func = mir_func_gen(p_h_func->p_sym);
    p_hir2mir_info p_info = hir2mir_info_gen(p_h_func->p_sym);
    p_mir_basic_block p_block = hir2mir_block_gen(p_info, p_h_func->p_block);
    mir_func_set_block(p_m_func, p_block);
    p_m_func->p_basic_block_list = p_info->p_basic_block_list;
    p_m_func->p_operand_list = p_info->p_operand_list;
    p_info->p_ret_operand->id = p_info->temp_id ++;
    p_m_func->p_ret_block = p_info->p_ret_block;
    // 为 return 语句设置编号
    p_m_func->p_ret_block->block_id = p_info->block_id ++;
    hir2mir_info_drop(p_info);
    return p_m_func;
}