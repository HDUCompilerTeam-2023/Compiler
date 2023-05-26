#ifndef __SYMBOL_GEN_FUNC__
#define __SYMBOL_GEN_FUNC__

#include <symbol/func.h>

p_symbol_func symbol_func_gen(const char *name, basic_type b_type);

void symbol_func_add_variable(p_symbol_func p_func, p_symbol_var p_var);
void symbol_func_add_param(p_symbol_func p_func, p_symbol_var p_var);

void symbol_func_drop(p_symbol_func p_func);

#endif
