#ifndef __HIR_GEN_SYMBOL__
#define __HIR_GEN_SYMBOL__

#include <hir/symbol.h>

p_symbol_init_list symbol_init_list_gen(void);
p_symbol_init_list symbol_init_list_add(p_symbol_init_list p_list, p_symbol_init p_init);
p_symbol_init symbol_init_gen_exp(p_hir_exp p_exp);
p_symbol_init symbol_init_gen_list(p_symbol_init_list p_list);
void symbol_init_drop(p_symbol_init p_init);

p_symbol_store symbol_store_initial();
void symbol_push_zone(p_symbol_store pss);
void symbol_pop_zone(p_symbol_store pss);
void symbol_store_destroy(p_symbol_store pss);

p_symbol_sym symbol_add(p_symbol_store pss, const char *name, p_symbol_type p_type, bool is_const, bool is_global, p_symbol_init p_init);
p_symbol_sym symbol_find(p_symbol_store pss, const char *name);

#endif