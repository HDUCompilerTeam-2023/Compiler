#ifndef __SYMBOL_VAR__
#define __SYMBOL_VAR__

#include <hir.h>
#include <mir.h>
#include <symbol.h>

struct symbol_init_val {
    union {
        INTCONST_t i;
        FLOATCONST_t f;
    };
};

struct symbol_init {
    basic_type basic;
    size_t size;
    p_symbol_init_val memory;
};

struct symbol_var {
    // type info
    p_symbol_type p_type;

    char *name;
    uint64_t id;

    p_symbol_init p_init;
    bool is_global;
    bool is_const;
    bool is_def;

    list_head node;
};

#endif
