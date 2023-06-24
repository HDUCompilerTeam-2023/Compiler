#ifndef __FRONTEND_SYNTAX_DECL_USE__
#define __FRONTEND_SYNTAX_DECL_USE__

#include <frontend/syntax/init/use.h>
#include <frontend/syntax/type/use.h>

typedef struct syntax_decl syntax_decl, *p_syntax_decl;

const char *syntax_decl_list_node_get_name(p_syntax_decl p_decl);
p_syntax_type_array syntax_decl_list_node_get_array(p_syntax_decl p_decl);
p_syntax_init syntax_decl_list_node_get_init(p_syntax_decl p_decl);
p_syntax_decl syntax_decl_arr(p_syntax_decl p_decl, p_ast_exp p_exp);
p_syntax_decl syntax_decl_init(p_syntax_decl p_decl, p_syntax_init p_init);

#endif
