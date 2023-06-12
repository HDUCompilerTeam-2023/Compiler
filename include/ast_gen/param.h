#ifndef __AST_GEN_PARAM__
#define __AST_GEN_PARAM__

#include <ast/param.h>

p_ast_param_list ast_param_list_init(void);
p_ast_param_list ast_param_list_add(p_ast_param_list p_head, p_ast_exp p_exp);
void ast_param_list_drop(p_ast_param_list p_param_list);

#endif
