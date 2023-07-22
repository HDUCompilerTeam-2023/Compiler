#ifndef __PROGRAM_GEN__
#define __PROGRAM_GEN__

#include <program/use.h>

#include <ir.h>

p_program program_gen(const char *input, const char *output);
void program_drop(p_program p_program);

bool program_add_str(p_program p_program, p_symbol_str p_str);
bool program_add_global(p_program p_program, p_symbol_var p_var);
bool program_add_function(p_program p_program, p_symbol_func p_func);

#endif
