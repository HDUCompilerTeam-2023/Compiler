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
    bool is_def;

    char *name;
    union {
        struct {
            p_hir_func p_func;
            list_head local;
        };
        struct {
            p_symbol_init p_init;
            uint64_t id;
            bool is_global;
        };
    };

    list_head node;
};

#endif