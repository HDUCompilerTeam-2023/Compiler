#ifndef __MIRFUNC__
#define __MIRFUNC__
#include <mir.h>

struct mir_func {
    p_symbol_sym p_func_sym;

    list_head entry_block;
    list_head vreg_list;
    list_head vmem_list;
};

#endif