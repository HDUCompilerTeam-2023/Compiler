#ifndef __FRONTEND_SYNTAX_DECL_DEF__
#define __FRONTEND_SYNTAX_DECL_DEF__

#include <frontend/syntax/decl/use.h>

struct syntax_decl {
    char *name;
    p_syntax_type_array p_array;
    p_syntax_init p_init;
};

#endif
