#ifndef __HIR2MIR_INFO_GEN__
#define __HIR2MIR_INFO_GEN__

#include <hir2mir/info.h>

#include <program/use.h>

struct hir2mir_info {
    p_mir_basic_block p_current_basic_block;
    p_symbol_func p_func;
    p_program p_program;

    p_mir_basic_block p_ret_block; // 返回的块
    p_symbol_var p_ret_vmem; // 返回值所存储的位置
};

p_hir2mir_info hir2mir_info_gen(p_symbol_func p_m_func, p_program p_program);

void hir2mir_info_add_instr(p_hir2mir_info p_info, p_mir_instr p_instr);
void hir2mir_info_add_basic_block(p_hir2mir_info p_info, p_mir_basic_block p_basic_block);

void hir2mir_info_drop(p_hir2mir_info p_info);
#include <mir_gen.h>
#endif
