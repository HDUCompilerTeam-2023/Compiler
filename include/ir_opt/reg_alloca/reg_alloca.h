#ifndef __IR_OPT_REG_ALLOCA_REG_ALLOC__
#define __IR_OPT_REG_ALLOCA_REG_ALLOC__
#include <util.h>
#include <program/use.h>

typedef enum alloca_type alloca_type;

enum alloca_type {
    alloca_min_spill,
    alloca_whole_in_mem,
    alloca_color_graph,
};

void reg_alloca_pass(alloca_type type, size_t reg_r_num, size_t reg_s_num, p_program p_program);
#endif