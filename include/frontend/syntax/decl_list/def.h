#ifndef __FRONTEND_SYNTAX_DECL_LIST_DEF__
#define __FRONTEND_SYNTAX_DECL_LIST_DEF__

#include <frontend/syntax/decl_list/use.h>

struct syntax_decl_list {
    list_head decl;
    bool is_const;
    basic_type type;
};

#endif
