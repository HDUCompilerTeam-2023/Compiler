#ifndef __AST_PARAM__
#define __AST_PARAM__

#include <ast.h>

struct ast_param_list {
    list_head param;
};

struct ast_param {
    p_ast_exp p_exp;
    list_head node;
};

#endif
