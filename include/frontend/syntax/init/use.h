#ifndef __FRONTEN_SYNTAX_INIT_USE__
#define __FRONTEN_SYNTAX_INIT_USE__

#include <util.h>
#include <ast/exp.h>

typedef struct syntax_init syntax_init, *p_syntax_init;

p_ast_exp syntax_init_get_exp(p_syntax_init p_init, p_symbol_type p_type, size_t offset);

#endif
