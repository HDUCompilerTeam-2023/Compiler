#ifndef __HIR_FUNC__
#define __HIR_FUNC__

#include <hir.h>

struct hir_func {
    p_hir_block p_block;
    p_symbol_sym p_sym;

    list_head node;
};

#endif