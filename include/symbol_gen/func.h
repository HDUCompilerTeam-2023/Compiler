#ifndef __SYMBOL_GEN_FUNC__
#define __SYMBOL_GEN_FUNC__

#include <symbol/func.h>

p_symbol_func symbol_func_gen(const char *name, basic_type b_type, p_symbol_type p_params);

void symbol_func_drop(p_symbol_func p_func);

#endif
