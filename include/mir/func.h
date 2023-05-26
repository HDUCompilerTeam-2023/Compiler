#ifndef __MIRFUNC__
#define __MIRFUNC__
#include <mir.h>

struct mir_func {
    p_symbol_func p_func_sym;

    list_head param_list;
    size_t param_cnt;

    list_head block;
    size_t block_cnt;
    list_head vreg_list;
    size_t vreg_cnt;
    list_head vmem_list;
    size_t vmem_cnt;
};

#endif
