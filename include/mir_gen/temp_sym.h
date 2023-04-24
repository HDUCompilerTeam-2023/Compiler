#ifndef __MIR_GEN_TEMP_SYM__
#define __MIR_GEN_TEMP_SYM__

#include <mir/temp_sym.h>

p_mir_temp_sym mir_temp_sym_basic_gen(basic_type b_type, p_mir_func p_func);
p_mir_temp_sym mir_temp_sym_pointer_gen(basic_type b_type, p_mir_func p_func);

void mir_temp_sym_drop(p_mir_temp_sym p_temp_sym);
#endif