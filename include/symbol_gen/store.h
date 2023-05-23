#ifndef __SYMBOL_GEN_STORE__
#define __SYMBOL_GEN_STORE__

#include <symbol/store.h>

#include <mir.h>

p_program symbol_store_gen(void);
void symbol_store_hir_drop(p_program p_store);
void symbol_store_mir_drop(p_program p_store);
void symbol_store_drop(p_program p_store);

bool symbol_store_add_str(p_program p_store, p_symbol_str p_str);
bool symbol_store_add_global(p_program p_store, p_symbol_sym p_sym);
bool symbol_store_add_local(p_program p_store, p_symbol_sym p_sym);
bool symbol_store_add_function(p_program p_store, p_symbol_sym p_sym);

void symbol_store_mir_vmem_add(p_program p_store, p_mir_vmem p_vmem);
void symbol_store_mir_set_vmem_id(p_program p_store);

#endif
