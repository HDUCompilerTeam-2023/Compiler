#ifndef __HIR_GEN_FUNC__
#define __HIR_GEN_FUNC__

#include <hir/func.h>

p_hir_func hir_func_gen(p_symbol_sym p_sym, p_hir_block p_block);
void hir_func_drop(p_hir_func p_func);

#endif
