#ifndef __MIR_GEN_PROGRAM__
#define __MIR_GEN_PROGRAM__
#include <mir/program.h>
p_mir_program mir_program_gen(size_t func_cnt);

p_mir_program mir_program_func_add(p_mir_program p_program, p_mir_func p_func);
void mir_program_vmem_add(p_mir_program p_program, p_mir_vmem p_vmem);
void mir_program_set_vmem_id(p_mir_program p_program);

void mir_program_drop(p_mir_program p_program);

#endif