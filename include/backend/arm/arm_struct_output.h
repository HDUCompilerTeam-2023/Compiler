#ifndef __BACKEND_ARM_INSTR_PRINT__
#define __BACKEND_ARM_INSTR_PRINT__
#include <backend/arm/arm_struct.h>
#include <stdio.h>
void arm_func_output(FILE *out_file, p_arm_func p_func);
void arm_global_sym_output(FILE *out_file, p_symbol_var p_sym);
#endif