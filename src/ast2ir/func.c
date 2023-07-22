#include <ast2ir.h>

#include <stdio.h>
#include <symbol_gen/func.h>
#include <symbol_gen/type.h>
#include <symbol_gen/var.h>

static inline void ast2symbol_func_param_gen(p_ast2ir_info p_info, p_symbol_func p_func) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->param) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);

        p_ir_vreg p_param = ir_vreg_gen(symbol_type_copy(p_var->p_type));
        symbol_func_param_reg_add(p_func, p_param);
        ast2ir_info_add_instr(p_info, ir_store_instr_gen(ir_operand_addr_gen(p_var, NULL, 0), ir_operand_vreg_gen(p_param), true));
    }
}
static inline void ast2symbol_func_retval_gen(p_ast2ir_info p_info, p_symbol_func p_func) {
    if (p_func->ret_type == type_void) return;
    p_symbol_var p_ret_vmem = symbol_temp_var_gen(symbol_type_var_gen(p_func->ret_type));
    symbol_func_add_variable(p_func, p_ret_vmem);
    p_info->p_ret_vmem = p_ret_vmem;
}

// 生成函数 ir 将 p_info 传来的信息回馈给 symbol_func
void ast2ir_symbol_func_gen(p_ast_block p_h_block, p_symbol_func p_m_func, p_program p_program) {
    assert(p_h_block);

    p_ast2ir_info p_info = ast2ir_info_gen(p_m_func, p_program);

    ast2symbol_func_param_gen(p_info, p_m_func);
    ast2symbol_func_retval_gen(p_info, p_m_func);

    ast2ir_block_gen(p_info, p_h_block);

    ast2ir_info_add_basic_block(p_info, p_info->p_ret_block);

    if (p_info->p_ret_vmem) {
        p_ir_vreg p_ret_val = ir_vreg_gen(symbol_type_copy(p_info->p_ret_vmem->p_type));
        ast2ir_info_add_instr(p_info, ir_load_instr_gen(ir_operand_addr_gen(p_info->p_ret_vmem, NULL, 0), p_ret_val, true));
        ir_basic_block_set_ret(p_info->p_current_basic_block, ir_operand_vreg_gen(p_ret_val));
    }
    else {
        ir_basic_block_set_ret(p_info->p_current_basic_block, NULL);
    }

    symbol_func_set_block_id(p_m_func);
    ast2ir_info_drop(p_info);
}
