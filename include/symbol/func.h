#ifndef __SYMBOL_FUNC__
#define __SYMBOL_FUNC__

#include <ir.h>
#include <symbol.h>

struct symbol_func {
    // type info
    bool is_va;
    basic_type ret_type;

    char *name;
    uint64_t id;

    size_t var_cnt;
    list_head param;
    list_head constant;
    list_head variable;

    list_head param_reg_list;
    list_head vreg_list;

    size_t block_cnt;
    list_head block;

    list_head node;
};

#endif
