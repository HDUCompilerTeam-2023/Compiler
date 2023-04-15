#ifndef __MIR_GEN_FUNC__
#define __MIR_GEN_FUNC__
#include <mir/func.h>

p_mir_func mir_func_gen(p_symbol_sym p_sym);
p_mir_func mir_func_set_block(p_mir_func p_func, p_mir_basic_block p_block);

void mir_func_drop(p_mir_func p_func);
#endif