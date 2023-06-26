#ifndef __FRONTEN_SYNTAX_INIT_DEF__
#define __FRONTEN_SYNTAX_INIT_DEF__

#include <frontend/syntax/init/use.h>

struct syntax_init {
    bool is_exp;
    bool syntax_const;
    union {
        p_ast_exp p_exp;
        list_head list;
    };

    list_head node;
};

#endif
