#include <hir2mir.h>
#include <program/def.h>
#include <symbol/type.h>
#include <symbol_gen/func.h>

// 根据函数信息 生成接下来需要收集的信息的返回值类型
p_hir2mir_info hir2mir_info_gen(p_symbol_func p_m_func, p_program p_program) {
    p_hir2mir_info p_info = malloc(sizeof(*p_info));
    *p_info = (hir2mir_info) {
        .p_current_basic_block = NULL,
        .p_func = p_m_func,
        .p_program = p_program,
        .p_ret_vmem = NULL,
        .p_ret_block = mir_basic_block_gen(),
    };

    p_mir_basic_block p_entry = mir_basic_block_gen();
    hir2mir_info_add_basic_block(p_info, p_entry);

    return p_info;
}

void hir2mir_info_drop(p_hir2mir_info p_info) {
    free(p_info);
}

void hir2mir_info_add_basic_block(p_hir2mir_info p_info, p_mir_basic_block p_new) {
    symbol_func_bb_add(p_info->p_func, p_new);
    p_info->p_current_basic_block = p_new;
}

void hir2mir_info_add_instr(p_hir2mir_info p_info, p_mir_instr p_instr) {
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_instr);
    p_mir_vreg p_des = mir_instr_get_des(p_instr);
    if (p_des) {
        symbol_func_vreg_add(p_info->p_func, p_des);
    }
}
