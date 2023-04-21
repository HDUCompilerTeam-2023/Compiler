#ifndef __HIR_GEN_SYMBOL_TABLE__
#define __HIR_GEN_SYMBOL_TABLE__

#include <hir/symbol_table.h>

p_symbol_table symbol_table_gen();
void symbol_table_drop(p_symbol_table p_table);

void symbol_table_zone_push(p_symbol_table p_table);
void symbol_table_zone_pop(p_symbol_table p_table);

p_symbol_item symbol_table_item_add(p_symbol_table p_table, p_symbol_sym p_sym);
p_symbol_item symbol_table_item_find(p_symbol_table p_table, const char *name);
p_symbol_str symbol_table_str_get(p_symbol_table p_table, const char *string);

#endif