#ifndef __PROGRAM_GEN__
#define __PROGRAM_GEN__

#include <program/use.h>

#include <mir.h>

p_program program_gen(void);
void program_hir_drop(p_program p_program);
void program_mir_drop(p_program p_program);
void program_drop(p_program p_program);

bool program_add_str(p_program p_program, p_symbol_str p_str);
bool program_add_global(p_program p_program, p_symbol_var p_var);
bool program_add_constant(p_program p_program, p_symbol_var p_var);
bool program_add_function(p_program p_program, p_symbol_func p_func);

void program_mir_vmem_add(p_program p_program, p_mir_vmem p_vmem);
void program_mir_vmem_del(p_program p_program, p_mir_vmem p_vmem);
void program_mir_set_vmem_id(p_program p_program);

#endif
