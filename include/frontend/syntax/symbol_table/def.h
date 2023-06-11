#ifndef __FRONTEND_SYNTAX_SYMBOL_TABLE_DEF__
#define __FRONTEND_SYNTAX_SYMBOL_TABLE_DEF__

#include <frontend/syntax/symbol_table/use.h>

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
    p_symbol_zone p_top_table;
};

#define hash_P (65537)
#define hash_MOD (109)

#endif
