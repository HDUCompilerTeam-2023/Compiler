#ifndef __MIRFUNC__
#define __MIRFUNC__
#include <mir.h>
struct mir_func{
    p_mir_basic_block basic_block;
    p_symbol_sym p_func;

    list_head node;
};

#endif