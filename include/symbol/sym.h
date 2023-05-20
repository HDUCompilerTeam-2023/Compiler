#ifndef __SYMBOL_SYM__
#define __SYMBOL_SYM__

#include <hir.h>
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

struct symbol_sym {
    // type info
    p_symbol_type p_type;

    char *name;
    uint64_t id;
    union {
        struct {
            p_list_head last_param;
            list_head variable;
        };
        struct {
            p_symbol_init p_init;
            bool is_global;
            bool is_const;
            bool is_def;
        };
    };

    list_head node;
};

#endif