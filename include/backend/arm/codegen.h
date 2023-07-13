#ifndef __BACKEND_ARM_CODEGEN__
#define __BACKEND_ARM_CODEGEN__
#include <program/use.h>
#include <symbol/func.h>
#include <util.h>
typedef struct arm_codegen_info arm_codegen_info, *p_arm_codegen_info;

struct arm_codegen_info {
    char *asm_code;
    size_t mov_num;
    size_t swap_num;
    p_symbol_func p_func;
};

void arm_codegen_pass(p_program p_mir, char *asm_code);

#endif