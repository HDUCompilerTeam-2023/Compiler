#ifndef __SYMBOL_GEN_SYM__
#define __SYMBOL_GEN_SYM__

#include <symbol/sym.h>

p_symbol_init symbol_init_gen(size_t size, basic_type basic);
p_symbol_init symbol_init_add(p_symbol_init p_init, size_t offset, symbol_init_val val);
void symbol_init_drop(p_symbol_init p_init);

p_symbol_sym symbol_var_gen(const char *name, p_symbol_type p_type, bool is_const, bool is_def, void *p_data);
p_symbol_sym symbol_func_gen(const char *name, p_symbol_type p_type);

void symbol_var_drop(p_symbol_sym p_sym);
void symbol_func_drop(p_symbol_sym p_sym);

#endif