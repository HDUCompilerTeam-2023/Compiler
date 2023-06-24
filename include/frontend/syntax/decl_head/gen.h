#ifndef __FRONTEND_SYNTAX_DECL_HEAD_GEN__
#define __FRONTEND_SYNTAX_DECL_HEAD_GEN__

#include <frontend/syntax/decl_head/use.h>

p_syntax_decl_head syntax_decl_head_gen(basic_type b_type, bool is_const);
void syntax_decl_head_drop(p_syntax_decl_head p_decl_list);

#endif
