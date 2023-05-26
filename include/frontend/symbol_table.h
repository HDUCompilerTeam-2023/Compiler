#ifndef __FRONTEND_SYMBOL_TABLE__
#define __FRONTEND_SYMBOL_TABLE__

#include <hir.h>
#include <symbol.h>

#include <program/use.h>

typedef struct symbol_item symbol_item, *p_symbol_item;
typedef struct symbol_name symbol_name, *p_symbol_name;
typedef struct symbol_zone symbol_zone, *p_symbol_zone;
typedef struct symbol_table symbol_table, *p_symbol_table;

struct symbol_item {
    p_symbol_name p_name;
    p_symbol_item p_prev;

    uint16_t level;
    p_symbol_item p_next;

    bool is_func;
    union {
        p_symbol_var p_var;
        p_symbol_func p_func;
    };
};

struct symbol_name {
    char *name;
    p_symbol_item p_item;

    size_t hash_tag;
    hlist_node node;
};

struct symbol_zone {
    p_symbol_item p_item;

    p_symbol_zone p_prev;
};

struct symbol_table {
    hlist_hash hash;
    hlist_hash string_hash;

    uint16_t level;
    uint16_t next_id;
    p_symbol_zone p_top_table;

    list_head constant;

    p_program p_program;
    p_symbol_func p_func;
};

p_symbol_table symbol_table_gen();
void symbol_table_drop(p_symbol_table p_table);

void symbol_table_zone_push(p_symbol_table p_table);
void symbol_table_zone_pop(p_symbol_table p_table);

p_symbol_item symbol_table_var_add(p_symbol_table p_table, p_symbol_var p_var);
p_symbol_item symbol_table_func_add(p_symbol_table p_table, p_symbol_func p_func);
void symbol_table_constant_add(p_symbol_table p_table, p_symbol_var p_var);
p_symbol_var symbol_table_var_find(p_symbol_table p_table, const char *name);
p_symbol_func symbol_table_func_find(p_symbol_table p_table, const char *name);
p_symbol_str symbol_table_str_get(p_symbol_table p_table, const char *string);

#endif
