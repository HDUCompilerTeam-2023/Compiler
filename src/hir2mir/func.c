#include <hir2mir.h>

#include <symbol_gen/type.h>
#include <symbol_gen/func.h>
#include <symbol_gen/var.h>
#include <stdio.h>

static inline void hir2symbol_func_param_gen(p_hir2mir_info p_info, p_symbol_func p_func) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->param) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);

        p_mir_vreg p_param = mir_vreg_gen(symbol_type_copy(p_var->p_type));
        symbol_func_param_reg_add(p_func, p_param);
        hir2mir_info_add_instr(p_info, mir_store_instr_gen(mir_operand_addr_gen(p_var), NULL, mir_operand_vreg_gen(p_param)));
    }
}
static inline void hir2symbol_func_retval_gen(p_hir2mir_info p_info, p_symbol_func p_func) {
    if (p_func->ret_type == type_void) return;
    p_symbol_var p_ret_vmem = symbol_temp_var_gen(symbol_type_var_gen(p_func->ret_type));
    symbol_func_add_variable(p_func, p_ret_vmem);
    p_info->p_ret_vmem = p_ret_vmem;
}

// 生成函数 mir 将 p_info 传来的信息回馈给 symbol_func
void hir2mir_symbol_func_gen(p_hir_block p_h_block, p_symbol_func p_m_func, p_program p_program) {
    assert(p_h_block);

    p_hir2mir_info p_info = hir2mir_info_gen(p_m_func, p_program);

    hir2symbol_func_param_gen(p_info, p_m_func);
    hir2symbol_func_retval_gen(p_info, p_m_func);

    hir2mir_block_gen(p_info, p_h_block);

    hir2mir_info_add_basic_block(p_info, p_info->p_ret_block);

    if (p_info->p_ret_vmem) {
        p_mir_vreg p_ret_val = mir_vreg_gen(symbol_type_copy(p_info->p_ret_vmem->p_type));
        hir2mir_info_add_instr(p_info, mir_load_instr_gen(mir_operand_addr_gen(p_info->p_ret_vmem), NULL, p_ret_val));
        mir_basic_block_set_ret(p_info->p_current_basic_block, mir_operand_vreg_gen(p_ret_val));
    }
    else {
        mir_basic_block_set_ret(p_info->p_current_basic_block, NULL);
    }

    symbol_func_set_block_id(p_m_func);
    symbol_func_set_vreg_id(p_m_func);
    hir2mir_info_drop(p_info);
}
