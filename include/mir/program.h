
#ifndef __MIR_PROGRAM__
#define __MIR_PROGRAM__
#include <mir.h>
struct mir_program{
    p_mir_func func_table;
    size_t func_cnt;

    p_symbol_store p_store;
};

#endif