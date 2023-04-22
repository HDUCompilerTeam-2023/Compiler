#include <hir2mir/info_gen.h>
#include <symbol/sym.h>
#include <symbol/type.h>

// 根据函数信息 生成接下来需要收集的信息的返回值类型
p_hir2mir_info hir2mir_info_gen(p_symbol_sym p_func_sym){
    p_hir2mir_info p_info = malloc(sizeof(*p_info));
    *p_info = (hir2mir_info){
        .p_current_basic_block = NULL,
        .p_operand_list = mir_operand_list_gen(),
        .p_basic_block_list = mir_basic_block_list_gen(),
        .p_ret_block = mir_basic_block_gen(),
    };
    p_info->p_ret_operand = mir_operand_temp_sym_basic_gen(p_info->temp_id, p_func_sym->p_type->basic);
    mir_operand_list_add(p_info->p_operand_list, p_info->p_ret_operand);

    p_mir_instr p_ret = mir_ret_instr_gen( p_info->p_ret_operand);
    mir_basic_block_addinstr(p_info->p_ret_block, p_ret);
    mir_basic_block_list_add(p_info->p_basic_block_list, p_info->p_ret_block);
    return p_info;
}

void hir2mir_info_drop(p_hir2mir_info p_info)
{
    free(p_info);
}
