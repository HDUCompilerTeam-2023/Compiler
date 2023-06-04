#include <hir2mir.h>

#include <symbol/func.h>
#include <symbol/var.h>
#include <symbol/type.h>
#include <symbol_gen/type.h>
#include <stdio.h>

static inline void hir2mir_func_param_gen(p_hir2mir_info p_info, p_mir_func p_func) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->p_func_sym->param) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_mir_vmem p_vmem = mir_vmem_sym_gen(p_var);
        mir_func_vmem_add(p_func, p_vmem);
        p_var->p_vmem = p_vmem;

        p_mir_vreg p_param = mir_vreg_gen(symbol_type_copy(p_vmem->p_type));
        mir_func_param_add(p_func, p_param);
        hir2mir_info_add_instr(p_info, mir_store_instr_gen(mir_operand_addr_gen(p_vmem), NULL, mir_operand_vreg_gen(p_param)));
    }
}
static inline void hir2mir_func_retval_gen(p_hir2mir_info p_info, p_mir_func p_func) {
    if (p_func->p_func_sym->ret_type == type_void) return;
    p_mir_vmem p_ret_vmem = mir_vmem_temp_gen(symbol_type_var_gen(p_func->p_func_sym->ret_type));
    mir_func_vmem_add(p_func, p_ret_vmem);
    p_info->p_ret_vmem = p_ret_vmem;
}

// 生成函数 mir 将 p_info 传来的信息回馈给 mir_func
void hir2mir_func_gen(p_symbol_func p_func, p_program p_program) {
    p_mir_func p_m_func = p_func->p_m_func = mir_func_gen();
    p_m_func->p_func_sym = p_func;

    if (!p_func->p_h_block) return;

    p_hir2mir_info p_info = hir2mir_info_gen(p_m_func, p_program);

    hir2mir_func_param_gen(p_info, p_m_func);
    hir2mir_func_retval_gen(p_info, p_m_func);

    hir2mir_block_gen(p_info, p_func->p_h_block);

    hir2mir_info_add_basic_block(p_info, p_info->p_ret_block);

    if (p_info->p_ret_vmem) {
        p_mir_vreg p_ret_val = mir_vreg_gen(symbol_type_copy(p_info->p_ret_vmem->p_type));
        hir2mir_info_add_instr(p_info, mir_load_instr_gen(mir_operand_addr_gen(p_info->p_ret_vmem), NULL, p_ret_val));
        mir_basic_block_set_ret(p_info->p_current_basic_block, mir_operand_vreg_gen(p_ret_val));
    }
    else {
        mir_basic_block_set_ret(p_info->p_current_basic_block, NULL);
    }

    mir_func_set_block_id(p_m_func);
    mir_func_set_vreg_id(p_m_func);
    mir_func_set_vmem_id(p_m_func);
    hir2mir_info_drop(p_info);
}
