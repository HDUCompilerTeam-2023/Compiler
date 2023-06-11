#ifndef __FRONTEN_SYNTAX_INIT_GEN__
#define __FRONTEN_SYNTAX_INIT_GEN__

#include <frontend/syntax/init/use.h>

p_syntax_init syntax_init_list_gen(void);
p_syntax_init syntax_init_exp_gen(p_hir_exp p_exp);
p_syntax_init syntax_init_list_add(p_syntax_init p_list, p_syntax_init p_init);

p_syntax_init_mem syntax_init_mem_gen(p_syntax_init p_init, p_symbol_type p_type);
void syntax_init_mem_drop(p_syntax_init_mem p_init);

#endif
