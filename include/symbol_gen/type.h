#ifndef __SYMBOL_GEN_TYPE__
#define __SYMBOL_GEN_TYPE__

#include <symbol/type.h>

p_symbol_type symbol_type_var_gen(basic_type basic);
p_symbol_type symbol_type_arrary_gen(size_t size);
p_symbol_type symbol_type_func_gen(bool is_va);
p_symbol_type symbol_type_param_gen(p_symbol_type p_param);

void symbol_type_drop(p_symbol_type p_type);

#endif