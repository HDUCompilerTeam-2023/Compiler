
#ifndef __MIR_PROGRAM__
#define __MIR_PROGRAM__
#include <mir.h>
struct mir_program{
    list_head func;

    p_symbol_store p_store;
};

#endif