#ifndef __BACKEND_ARM_CODEGEN__
#define __BACKEND_ARM_CODEGEN__
#include <program/use.h>
#include <symbol/func.h>
#include <util.h>
#include <stdio.h>
typedef struct arm_codegen_info arm_codegen_info, *p_arm_codegen_info;

struct arm_codegen_info {
    FILE *out_file;
    size_t mov_num;
    size_t swap_num;
    size_t *save_reg_r;
    size_t *save_reg_s;
    size_t save_reg_r_num;
    size_t save_reg_s_num;
    p_symbol_func p_func;
};

void arm_codegen_pass(p_program p_mir);

#endif