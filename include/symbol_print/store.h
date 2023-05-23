#ifndef __SYMBOL_PRINT_STORE__
#define __SYMBOL_PRINT_STORE__

#include <symbol.h>

void symbol_store_print(p_program p_store);
void symbol_store_hir_print(p_program p_store);
void symbol_store_mir_print(p_program p_store);
void symbol_store_mir_dom_info_print(p_program p_store);

#endif
