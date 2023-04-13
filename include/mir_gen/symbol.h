#ifndef __MIR_GEN_SYMBOL__
#define __MIR_GEN_SYMBOL__
#include <mir/symbol.h>
p_mir_symbol mir_declared_sym_gen(p_symbol_sym p_sym);
p_mir_symbol mir_temp_sym_gen(p_mir_func p_func, p_symbol_type p_type);

void mir_symbol_drop(p_mir_symbol p_sym);
#endif