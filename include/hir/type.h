#ifndef __HIR_TYPE__
#define __HIR_TYPE__

#include <hir.h>

struct symbol_type {
    enum {
        type_var, type_arrary,
        type_func, type_param,
    } kind;
    union {
        p_symbol_type p_item;
        basic_type basic;
    };
    union {
        size_t size;
        p_symbol_type p_params;
    };
};

struct type {
    enum {
        tval, tarr,
        tfunc, tparam,
    };
    union {
        struct type * p_son;
        basic_type basic;
    };
    struct type * params;
};

#endif
