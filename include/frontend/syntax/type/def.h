#ifndef __FRONTEND_SYNTAX_TYPE_DEF__
#define __FRONTEND_SYNTAX_TYPE_DEF__

#include <frontend/syntax/type/use.h>

struct syntax_type_array {
    uint64_t size;
    p_syntax_type_array p_prev;
};

#endif
