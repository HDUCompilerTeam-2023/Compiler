#ifndef __FRONTEND_SYNTAX_DECL_LIST_NODE_DEF__
#define __FRONTEND_SYNTAX_DECL_LIST_NODE_DEF__

#include <frontend/syntax/decl_list/node/use.h>

struct syntax_decl {
    char *name;
    basic_type b_type;
    p_syntax_type_array p_array;
    p_syntax_init p_init;

    list_head node;
};

#endif
