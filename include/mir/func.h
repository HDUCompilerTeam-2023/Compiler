#ifndef __MIRFUNC__
#define __MIRFUNC__
#include <mir.h>

struct mir_func {
    p_symbol_sym p_func_sym;

    p_mir_vreg *param_vreg;
    size_t param_vreg_cnt;

    list_head entry_block;
    list_head vreg_list;
    list_head vmem_list;
};

#endif
