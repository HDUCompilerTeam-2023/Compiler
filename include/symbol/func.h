#ifndef __SYMBOL_FUNC__
#define __SYMBOL_FUNC__

#include <hir.h>
#include <mir.h>
#include <symbol.h>

struct symbol_func {
    // type info
    bool is_va;
    basic_type ret_type;

    char *name;
    uint64_t id;

    list_head param;
    size_t param_cnt;
    list_head constant;
    size_t constant_cnt;
    list_head variable;
    size_t variable_cnt;

    p_hir_block p_h_block;
    p_mir_func p_m_func;

    list_head node;
};

#endif
