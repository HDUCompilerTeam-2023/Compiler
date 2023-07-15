#ifndef __FRONTEN_SYNTAX_INIT_USE__
#define __FRONTEN_SYNTAX_INIT_USE__

#include <util.h>
#include <ast/exp.h>

typedef struct syntax_init syntax_init, *p_syntax_init;

p_syntax_init syntax_init_get_entry(p_list_head p_node);
p_list_head syntax_init_get_head(p_syntax_init p_init);
p_ast_exp syntax_init_get_exp(p_syntax_init p_init);

p_ast_exp syntax_init_find_exp(p_syntax_init p_init, p_symbol_type p_type, size_t offset);

#endif
