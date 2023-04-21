#ifndef __SYMBOL_GEN_STR__
#define __SYMBOL_GEN_STR__

#include <symbol/str.h>
#include <hir.h>

p_symbol_str symbol_str_gen(const char *string);
void symbol_str_drop(p_symbol_str p_str);

#endif