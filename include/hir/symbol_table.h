#ifndef __HIR_SYMBOL_TABLE__
#define __HIR_SYMBOL_TABLE__

#include <hir.h>
#include <symbol.h>

typedef struct symbol_item symbol_item, *p_symbol_item;
typedef struct symbol_name symbol_name, *p_symbol_name;
typedef struct symbol_zone symbol_zone, *p_symbol_zone;
typedef struct symbol_table symbol_table, *p_symbol_table;

struct symbol_item {
    p_symbol_name p_name;
    p_symbol_item p_prev;

    uint16_t level;
    p_symbol_item p_next;

    p_symbol_sym p_info;
    union {
        p_hir_func p_func;
    };
};

struct symbol_name {
    char * name;
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
};

#endif