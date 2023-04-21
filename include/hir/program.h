#ifndef __HIR_PROGRAM__
#define __HIR_PROGRAM__

#include <hir.h>
#include <hir/symbol_table.h>

struct hir_program {
    // list_head init;
    p_symbol_table p_table;

    list_head func;

    p_symbol_store p_store;
};

#endif