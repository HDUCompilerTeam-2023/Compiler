#ifndef __HIR_SYMBOL__
#define __HIR_SYMBOL__

#include <hir.h>

typedef struct symbol_item symbol_item, *p_symbol_item;
typedef struct symbol_name symbol_name, *p_symbol_name;
typedef struct symbol_table symbol_table, *p_symbol_table;

struct symbol_item {
    p_symbol_name p_name;
    p_symbol_item p_prev;

    uint16_t level;
    p_symbol_item p_next;

    p_symbol_sym p_info;
};

struct symbol_name {
    char * name;
    p_symbol_item p_item;

    size_t hash_tag;
    hlist_node node;
};

struct symbol_table {
    p_symbol_item p_item;

    p_symbol_table p_prev;
};

struct symbol_store {
    hlist_hash hash;

    p_symbol_sym p_info;

    uint16_t level;
    p_symbol_table p_top_table;
};

struct symbol_init {
    size_t size;
    p_hir_exp *memory;
};

struct symbol_sym {
    // type info
    p_symbol_type p_type;
    bool is_const;
    // store info
    bool is_global;

    char *name;
    uint64_t id;
    union {
        p_hir_func p_func;
        p_symbol_init p_init;
    };

    p_symbol_sym p_next;
};

#endif