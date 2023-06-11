#ifndef __FRONTEND_SYNTAX_SYMBOL_TABLE_GEN__
#define __FRONTEND_SYNTAX_SYMBOL_TABLE_GEN__

#include <frontend/syntax/symbol_table/use.h>

p_symbol_table symbol_table_gen();
void symbol_table_drop(p_symbol_table p_table);

#endif
