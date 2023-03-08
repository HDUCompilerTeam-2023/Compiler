#ifndef __HIR_SYMBOL__
#define __HIR_SYMBOL__

#include <util.h>

typedef struct symbol_info symbol_info, *p_symbol_info;
typedef struct symbol_item symbol_item, *p_symbol_item;
typedef struct symbol_name symbol_name, *p_symbol_name;
typedef struct symbol_table symbol_table, *p_symbol_table;
typedef struct symbol_store symbol_store, *p_symbol_store;

struct symbol_info {
    p_symbol_info p_next;
};

p_symbol_store symbol_store_initial();
void symbol_push_zone(p_symbol_store pss);
void symbol_pop_zone(p_symbol_store pss);
void symbol_store_destroy(p_symbol_store pss);

bool symbol_add(p_symbol_store pss, const char *name);
p_symbol_info symbol_find(p_symbol_store pss, const char *name);

#endif