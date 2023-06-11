#ifndef __FRONTEND_SYMBOL_TABLE__
#define __FRONTEND_SYMBOL_TABLE__

#include <symbol.h>

typedef struct symbol_item symbol_item, *p_symbol_item;
typedef struct symbol_name symbol_name, *p_symbol_name;
typedef struct symbol_zone symbol_zone, *p_symbol_zone;
typedef struct symbol_table symbol_table, *p_symbol_table;

void symbol_table_zone_push(p_symbol_table p_table);
void symbol_table_zone_pop(p_symbol_table p_table);

p_symbol_item symbol_table_var_add(p_symbol_table p_table, p_symbol_var p_var);
p_symbol_item symbol_table_func_add(p_symbol_table p_table, p_symbol_func p_func);
p_symbol_var symbol_table_var_find(p_symbol_table p_table, const char *name);
p_symbol_func symbol_table_func_find(p_symbol_table p_table, const char *name);
p_symbol_str symbol_table_str_find(p_symbol_table p_table, const char *string);
p_symbol_str symbol_table_str_add(p_symbol_table p_table, const char *string);

#endif
