#ifndef __HIR_SYMBOL__
#define __HIR_SYMBOL__

#include <hir.h>

struct symbol_init_list {
    list_head init;
    bool syntax_const;
};
struct symbol_init {
    bool is_exp;
    bool syntax_const;
    union {
        p_hir_exp p_exp;
        p_symbol_init_list p_list;
    };

    list_head node;
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