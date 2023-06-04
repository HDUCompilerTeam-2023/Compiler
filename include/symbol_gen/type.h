#ifndef __SYMBOL_GEN_TYPE__
#define __SYMBOL_GEN_TYPE__

#include <symbol/type.h>

void symbol_type_push_array(p_symbol_type p_type, p_symbol_type_array p_array);
p_symbol_type_array symbol_type_top_array(p_symbol_type p_type);
p_symbol_type_array symbol_type_pop_array(p_symbol_type p_type);

void symbol_type_push_ptr(p_symbol_type p_type);
void symbol_type_pop_ptr(p_symbol_type p_type);

p_symbol_type symbol_type_copy(p_symbol_type p_type);
p_symbol_type_array symbol_type_array_copy(p_symbol_type_array p_array);

p_symbol_type symbol_type_var_gen(basic_type basic);
p_symbol_type_array symbol_type_array_gen(size_t size);

void symbol_type_drop(p_symbol_type p_type);
void symbol_type_array_drop(p_symbol_type_array p_array);

#endif