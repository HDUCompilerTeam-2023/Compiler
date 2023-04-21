#ifndef __SYMBOL_TYPE__
#define __SYMBOL_TYPE__

#include <symbol.h>

struct symbol_type {
    enum {
        type_arrary, type_var,
        type_param,  type_func,
                     type_va_func,
    } kind;
    union {
        p_symbol_type p_item;
        basic_type basic;
    };
    union {
        uint64_t size;
        p_symbol_type p_params;
    };
};

#endif