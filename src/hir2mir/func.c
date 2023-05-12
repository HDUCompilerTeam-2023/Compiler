#include <hir/func.h>
#include <hir2mir.h>

#include <symbol/sym.h>
#include <symbol/type.h>

// 生成函数 mir 将 p_info 传来的信息回馈给 mir_func
void hir2mir_func_gen(p_hir_func p_h_func, p_hir2mir_program_info p_program_info) {
    p_mir_func p_m_func = p_program_info->func_table + p_h_func->p_sym->id;
    p_m_func->p_func_sym = p_h_func->p_sym;

    if (!p_h_func->p_block) return;

    p_hir2mir_info p_info = hir2mir_info_gen(p_m_func, p_program_info);

    hir2mir_block_gen(p_info, p_h_func->p_block);

    list_replace(&p_info->p_ret_block->node, &p_info->p_current_basic_block->node);
    mir_basic_block_drop(p_info->p_current_basic_block);
    p_info->p_current_basic_block = p_info->p_ret_block;

    p_mir_vreg p_ret_addr = mir_vreg_gen(p_info->p_ret_vmem->b_type, p_info->p_ret_vmem->ref_level + 1);
    p_mir_vreg p_ret_val = mir_vreg_gen(p_info->p_ret_vmem->b_type, p_info->p_ret_vmem->ref_level);
    hir2mir_info_add_instr(p_info, mir_addr_instr_gen(p_info->p_ret_vmem, p_ret_addr));
    hir2mir_info_add_instr(p_info, mir_load_instr_gen(mir_operand_vreg_gen(p_ret_addr), NULL, p_ret_val));
    hir2mir_info_add_instr(p_info, mir_ret_instr_gen(mir_operand_vreg_gen(p_ret_val)));

    mir_func_vmem_add(p_m_func, p_info->p_ret_vmem);

    mir_func_set_block_id(p_m_func);
    mir_func_set_vreg_id(p_m_func);
    mir_func_set_vmem_id(p_m_func);
    hir2mir_info_drop(p_info);
}
