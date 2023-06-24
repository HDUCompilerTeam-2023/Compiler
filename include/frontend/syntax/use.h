#ifndef __FRONTEN_SYNTAX__
#define __FRONTEN_SYNTAX__

typedef void *yyscan_t;

#include <frontend/syntax/info/use.h>

#include <frontend/syntax/init/use.h>

#include <frontend/syntax/decl_head/use.h>
#include <frontend/syntax/decl/use.h>

#include <frontend/syntax/type/use.h>

#include <ast.h>

p_ast_exp syntax_val_offset(p_ast_exp p_val, p_ast_exp p_offset);

p_ast_stmt syntax_return(p_syntax_info p_info, p_ast_exp p_exp);

#endif
