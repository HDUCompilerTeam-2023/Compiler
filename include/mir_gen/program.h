#ifndef __MIR_GEN_PROGRAM__
#define __MIR_GEN_PROGRAM__
#include <mir/program.h>
p_mir_program mir_program_gen(void);

p_mir_program mir_func_add(p_mir_program p_program, p_mir_func p_func);
p_mir_program mir_global_set(p_mir_program p_program, p_list_head p_head);

void mir_program_drop(p_mir_program p_program);

#endif