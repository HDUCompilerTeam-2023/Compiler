#include <hir2mir/info_gen.h>
#include <stdio.h>
#include <symbol/sym.h>
#include <symbol/type.h>

// 根据函数信息 生成接下来需要收集的信息的返回值类型
p_hir2mir_info hir2mir_info_gen(p_symbol_sym p_func_sym){
    p_hir2mir_info p_info = malloc(sizeof(*p_info));
    *p_info = (hir2mir_info){
        .p_current_basic_block = NULL,
        .p_operand_list = mir_operand_list_gen(),
        .p_ret_block = mir_basic_block_gen(),
        .temp_id = 0,
    };
    p_info->p_ret_operand = mir_operand_temp_sym_basic_gen(p_info->temp_id, p_func_sym->p_type->basic);
    mir_operand_list_add(p_info->p_operand_list, p_info->p_ret_operand);

    p_mir_instr p_ret = mir_ret_instr_gen( p_info->p_ret_operand);
    mir_basic_block_addinstr(p_info->p_ret_block, p_ret);
    return p_info;
}

void hir2mir_info_drop(p_hir2mir_info p_info)
{
    free(p_info);
}

void hir2mir_info_add_basic_block(p_hir2mir_info p_info, p_mir_basic_block p_new) {
    if (p_info->p_current_basic_block) {
        p_new->block_id = p_info->p_current_basic_block->block_id + 1;
        p_info->p_current_basic_block->p_next = p_new;
    }
    else {
        p_new->block_id = 0;
    }
    p_info->p_current_basic_block = p_new;
}
p_mir_basic_block hir2mir_info_get_current_block(p_hir2mir_info p_info) {
    return p_info->p_current_basic_block;
}
void hir2mir_info_add_instr(p_hir2mir_info p_info, p_mir_instr p_instr) {
    mir_basic_block_addinstr(hir2mir_info_get_current_block(p_info), p_instr);
}