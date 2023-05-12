#ifndef __MIR_GEN_FUNC__
#define __MIR_GEN_FUNC__
#include <mir/func.h>

p_mir_func mir_func_table_gen(size_t cnt);

void mir_func_add_basic_block(p_mir_func p_func, p_mir_basic_block p_basic_block);
void mir_func_vreg_add(p_mir_func p_func, p_mir_vreg p_vreg);
void mir_func_vreg_add_at(p_mir_func p_func, p_mir_vreg p_new_sym, p_mir_basic_block p_current_block, p_mir_instr p_instr);
void mir_func_vmem_add(p_mir_func p_func, p_mir_vmem p_vmem);

void mir_func_set_block_id(p_mir_func p_func);
void mir_func_set_vreg_id(p_mir_func p_func);
void mir_func_set_vmem_id(p_mir_func p_func);

void mir_func_table_drop(p_mir_func p_func, size_t cnt);
#endif