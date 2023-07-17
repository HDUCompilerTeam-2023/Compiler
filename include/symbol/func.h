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

    size_t param_reg_cnt;
    list_head param_reg_list;

    size_t vreg_cnt;
    list_head vreg_list;

    size_t block_cnt;
    list_head block;

    list_head call_param_vmem_list;
    size_t stack_size;
    size_t inner_stack_size;

    size_t instr_num;

    size_t use_reg_num_r;
    size_t use_reg_num_s;
    size_t save_reg_r_num;
    size_t save_reg_s_num;
    list_head node;
};

#endif
