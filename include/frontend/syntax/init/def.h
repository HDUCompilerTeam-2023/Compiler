#ifndef __FRONTEN_SYNTAX_INIT_DEF__
#define __FRONTEN_SYNTAX_INIT_DEF__

#include <frontend/syntax/init/use.h>

struct syntax_init {
    bool is_exp;
    bool syntax_const;
    union {
        p_hir_exp p_exp;
        list_head list;
    };

    list_head node;
};
struct syntax_init_mem {
    size_t size;
    p_hir_exp *memory;
};

#endif
