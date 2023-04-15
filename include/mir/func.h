#ifndef __MIRFUNC__
#define __MIRFUNC__
#include <mir.h>
struct mir_func{
    p_mir_basic_block p_basic_block;
    p_symbol_sym p_func_sym;

    list_head node;
};

#endif