#ifndef __MIR_GEN_FUNC__
#define __MIR_GEN_FUNC__
#include <mir/func.h>

p_mir_func mir_func_table_gen(size_t cnt);

void mir_func_add_basic_block(p_mir_func p_func, p_mir_basic_block p_basic_block);
void mir_func_temp_sym_add(p_mir_func p_func, p_mir_temp_sym p_temp_sym);

void mir_func_set_block_id(p_mir_func p_func);
void mir_func_set_temp_id(p_mir_func p_func);

void mir_func_table_drop(p_mir_func p_func, size_t cnt);
#endif