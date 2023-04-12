#ifndef __HIR_GEN_SYMBOL__
#define __HIR_GEN_SYMBOL__

#include <hir/symbol.h>

p_symbol_init symbol_init_gen(size_t size);
void symbol_init_drop(p_symbol_init p_init);

p_symbol_sym symbol_var_gen(const char *name, p_symbol_type p_type, bool is_const, bool is_def, void *p_data);
p_symbol_sym symbol_func_gen(const char *name, p_symbol_type p_type, bool is_const, bool is_def, void *p_data);

void symbol_var_drop(p_symbol_sym p_sym);
void symbol_func_drop(p_symbol_sym p_sym);

p_symbol_str symbol_str_gen(const char *string);
void symbol_str_drop(p_symbol_str p_str);

#endif