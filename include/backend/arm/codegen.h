#ifndef __BACKEND_ARM_CODEGEN__
#define __BACKEND_ARM_CODEGEN__
#include <util.h>
#include <program/use.h>
#include <symbol/func.h>
typedef struct arm_codegen_info arm_codegen_info, *p_arm_codegen_info;

struct arm_codegen_info {
    char *asm_code;
    char extra_code[10000];
    p_symbol_func p_func;
    size_t extra_len;
    size_t *mem_stack_offset;
    size_t stack_size;
};

void arm_codegen_pass(p_program p_mir, char *asm_code);

#endif