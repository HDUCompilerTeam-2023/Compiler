#ifndef __HIR_PROGRAM__
#define __HIR_PROGRAM__

#include <hir.h>

struct hir_program {
    // list_head init;
    p_symbol_store pss;
    list_head func;
};

#endif