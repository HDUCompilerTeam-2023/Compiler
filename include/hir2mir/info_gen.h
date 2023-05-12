#ifndef __HIR2MIR_INFO_GEN__
#define __HIR2MIR_INFO_GEN__

#include <hir2mir/info.h>

struct hir2mir_program_info {
    p_mir_program p_program;
    p_mir_func func_table;

    p_mir_vmem *global_vmem_table;
    size_t global_vmem_cnt;
};

struct hir2mir_info {
    p_mir_basic_block p_current_basic_block;
    p_mir_func p_func;

    p_mir_basic_block p_ret_block; // 返回的块
    p_mir_vmem p_ret_vmem; // 返回值所存储的位置

    p_mir_vreg *local_addr_table;
    size_t local_addr_cnt;

    p_mir_vreg *global_addr_table;
    size_t global_addr_cnt;

    p_hir2mir_program_info p_program_info;
};

p_hir2mir_program_info hir2mir_program_info_gen(p_mir_program p_m_program);
p_hir2mir_info hir2mir_info_gen(p_mir_func p_m_func, p_hir2mir_program_info p_program_info);

p_mir_instr hir2mir_info_add_br_instr(p_hir2mir_info p_info, p_mir_basic_block p_next);
void hir2mir_info_add_instr(p_hir2mir_info p_info, p_mir_instr p_instr);
void hir2mir_info_add_basic_block(p_hir2mir_info p_info, p_mir_basic_block p_basic_block);

void hir2mir_program_info_drop(p_hir2mir_program_info p_program_info);
void hir2mir_info_drop(p_hir2mir_info p_info);
#include <mir_gen.h>
#endif
