#ifndef __FRONTEN_SYNTAX_INIT_USE__
#define __FRONTEN_SYNTAX_INIT_USE__

#include <util.h>
#include <hir/exp.h>

typedef struct syntax_init syntax_init, *p_syntax_init;

typedef struct syntax_init_mem syntax_init_mem, *p_syntax_init_mem;

p_hir_exp syntax_init_mem_get_exp(p_syntax_init_mem p_init_mem, size_t offset);
void syntax_init_mem_clear_exp(p_syntax_init_mem p_init_mem, size_t offset);

#endif
