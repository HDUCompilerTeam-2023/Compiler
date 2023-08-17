#ifndef __IR_MANAGER_MEM_SSA__
#define __IR_MANAGER_MEM_SSA__

#include <program/use.h>
#include <symbol.h>

void memssa_func_pass(p_symbol_func p_func, p_program p_program);
void memssa_program_pass(p_program p_program);
void memssa_program_out(p_program p_ir);

#endif
