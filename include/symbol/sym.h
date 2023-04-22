#ifndef __SYMBOL_SYM__
#define __SYMBOL_SYM__

#include <symbol.h>
#include <hir.h>

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

struct symbol_sym {
    // type info
    p_symbol_type p_type;

    char *name;
    union {
        struct {
            list_head variable;
            list_head constant;
            uint64_t next_id;
        };
        struct {
            p_symbol_init p_init;
            uint64_t id;
            bool is_global;
            bool is_const;
            bool is_def;
        };
    };

    list_head node;
};

#endif