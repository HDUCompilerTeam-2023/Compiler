#ifndef __AST_GEN_STMT__
#define __AST_GEN_STMT__

#include <ast/stmt.h>

p_ast_stmt ast_stmt_return_gen(p_ast_exp p_exp);
p_ast_stmt ast_stmt_exp_gen(p_ast_exp p_exp);
p_ast_stmt ast_stmt_break_gen(void);
p_ast_stmt ast_stmt_continue_gen(void);
p_ast_stmt ast_stmt_if_gen(p_ast_exp p_exp, p_ast_stmt p_stmt_1);
p_ast_stmt ast_stmt_if_else_gen(p_ast_exp p_exp, p_ast_stmt p_stmt_1, p_ast_stmt p_stmt_2);
p_ast_stmt ast_stmt_while_gen(p_ast_exp p_exp, p_ast_stmt p_stmt_1);
p_ast_stmt ast_stmt_do_while_gen(p_ast_exp p_exp, p_ast_stmt p_stmt_1);
p_ast_stmt ast_stmt_block_gen(p_ast_block p_block);
p_ast_stmt ast_stmt_assign_gen(p_ast_exp lval, p_ast_exp rval);
void ast_stmt_drop(p_ast_stmt p_stmt);

#endif
