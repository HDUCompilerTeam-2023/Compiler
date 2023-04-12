#ifndef __HIR_GEN_SYMBOL_STORE__
#define __HIR_GEN_SYMBOL_STORE__

#include <hir/symbol_store.h>

p_symbol_store symbol_store_initial();
void symbol_store_destroy(p_symbol_store pss);
void symbol_push_zone(p_symbol_store pss);
void symbol_pop_zone(p_symbol_store pss);

bool symbol_add(p_symbol_store pss, p_symbol_sym p_sym);
p_symbol_sym symbol_find(p_symbol_store pss, const char *name);

#endif