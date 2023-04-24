#ifndef __SYMBOL_GEN_STORE__
#define __SYMBOL_GEN_STORE__

#include <symbol/store.h>

p_symbol_store symbol_store_gen(void);
void symbol_store_drop(p_symbol_store p_store);

bool symbol_store_add_str(p_symbol_store p_store, p_symbol_str p_str);
bool symbol_store_add_global(p_symbol_store p_store, p_symbol_sym p_sym);
bool symbol_store_add_local(p_symbol_store p_store, p_symbol_sym p_sym);
bool symbol_store_add_function(p_symbol_store p_store, p_symbol_sym p_sym);

#endif