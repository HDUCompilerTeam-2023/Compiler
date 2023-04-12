#ifndef __MIRFUNC__
#define __MIRFUNC__
#include <mir.h>
struct mir_func{
    p_mir_basic_block p_basic_block;
    p_symbol_sym p_func_sym;
    size_t temp_id; // 临时变量数量

    list_head node;
};

#endif