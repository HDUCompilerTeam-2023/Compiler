#ifndef __AST__
#define __AST__

#include <symbol.h>
#include <util.h>

typedef struct ast_param ast_param, *p_ast_param;
typedef struct ast_param_list ast_param_list, *p_ast_param_list;
typedef struct ast_func ast_func, *p_ast_func;
typedef struct ast_block ast_block, *p_ast_block;
typedef struct ast_stmt ast_stmt, *p_ast_stmt;
typedef struct ast_exp ast_exp, *p_ast_exp;
typedef enum ast_exp_binary_op ast_exp_binary_op;
typedef enum ast_exp_unary_op ast_exp_unary_op;
typedef enum ast_exp_logic_op ast_exp_logic_op;
typedef enum ast_exp_ulogic_op ast_exp_ulogic_op;

#endif
