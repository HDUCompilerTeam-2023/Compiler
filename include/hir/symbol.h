#ifndef __HIR_SYMBOL__
#define __HIR_SYMBOL__

#include <hir.h>

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
    union {
        p_symbol_init p_init;
    };

    p_symbol_sym p_next;
};

#endif