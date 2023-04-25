#include <hir2mir/info_gen.h>
#include <symbol/sym.h>
#include <symbol/type.h>

// 根据函数信息 生成接下来需要收集的信息的返回值类型
p_hir2mir_info hir2mir_info_gen(p_mir_func p_m_func, p_mir_func m_func_table){
    p_hir2mir_info p_info = malloc(sizeof(*p_info));
    *p_info = (hir2mir_info){
        .p_current_basic_block = NULL,
        .p_func = p_m_func,
        .func_table = m_func_table,
        .p_ret_block = mir_basic_block_gen(),
    };
    p_mir_temp_sym p_temp_sym =  mir_temp_sym_basic_gen(p_m_func->p_func_sym->p_type->basic, p_m_func);
    p_info->p_ret_operand = mir_operand_temp_sym_gen(p_temp_sym);

    p_mir_instr p_ret = mir_ret_instr_gen(p_info->p_ret_operand);
    mir_basic_block_addinstr(p_info->p_ret_block, p_ret);
    return p_info;
}

void hir2mir_info_drop(p_hir2mir_info p_info)
{
    free(p_info);
}

void hir2mir_info_add_basic_block(p_hir2mir_info p_info, p_mir_basic_block p_new) {
    mir_func_add_basic_block(p_info->p_func, p_new);
    p_info->p_current_basic_block = p_new;
}
void hir2mir_info_add_instr(p_hir2mir_info p_info, p_mir_instr p_instr) {
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_instr);
}