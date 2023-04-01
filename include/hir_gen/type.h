#ifndef __HIR_GEN_TYPE__
#define __HIR_GEN_TYPE__

#include <hir/type.h>

p_symbol_type symbol_type_var_gen(basic_type basic);
p_symbol_type symbol_type_arrary_gen(size_t size);
p_symbol_type symbol_type_func_gen(void);
p_symbol_type symbol_type_param_gen(p_symbol_type p_param);

void symbol_type_drop(p_symbol_type p_type);
void symbol_type_print(p_symbol_type p_type);

#endif