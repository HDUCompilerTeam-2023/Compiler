#ifndef __GRAMMAR__
#define __GRAMMAR__

#include <hir_gen/parser.h>
#include <hir_gen/lexer.h>

p_hir_program hir_gen(const char *file_name);
void hir_drop(p_hir_program p_hir);

#endif