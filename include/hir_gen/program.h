#ifndef __HIR_GEN_PROGRAM__
#define __HIR_GEN_PROGRAM__

#include <hir/program.h>

p_hir_program hir_program_gen(void);
void hir_program_drop(p_hir_program p_program);

bool hir_symbol_sym_add(p_hir_program p_program, p_symbol_sym p_sym);
p_symbol_sym hir_symbol_sym_find(p_hir_program p_program, const char *name);
p_symbol_str hir_symbol_str_get(p_hir_program p_program, const char *string);

void hir_symbol_zone_push(p_hir_program p_program);
void hir_symbol_zone_pop(p_hir_program p_program);

#endif