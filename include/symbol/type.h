#ifndef __SYMBOL_TYPE__
#define __SYMBOL_TYPE__

#include <symbol.h>

struct symbol_type {
    enum {
        type_arrary,
        type_var,
    } kind;
    union {
        p_symbol_type p_item;
        basic_type basic;
    };
    uint64_t size;
};

#endif