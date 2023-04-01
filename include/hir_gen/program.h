#ifndef __HIR_GEN_PROGRAM__
#define __HIR_GEN_PROGRAM__

#include <hir/program.h>

p_hir_program hir_program_gen(void);
p_hir_program hir_program_add(p_hir_program p_program, p_hir_func p_func);
void hir_program_drop(p_hir_program p_program);

#endif