#ifndef __AST2IR_STMT__
#define __AST2IR_STMT__

#include <ast2ir/info_gen.h>
void ast2ir_stmt_gen(p_ast2ir_info p_info, p_ir_basic_block while_start, p_ir_basic_block while_end_next, p_ast_stmt p_stmt);

void ast2ir_stmt_return_gen(p_ast2ir_info p_info, p_ast_exp p_exp);
void ast2ir_stmt_exp_gen(p_ast2ir_info p_info, p_ast_exp p_exp);
void ast2ir_stmt_break_gen(p_ast2ir_info p_info, p_ir_basic_block p_while_end_next);
void ast2ir_stmt_continue_gen(p_ast2ir_info p_info, p_ir_basic_block p_while_start);
void ast2ir_stmt_if_gen(p_ast2ir_info p_info, p_ir_basic_block p_while_start, p_ir_basic_block p_while_end_next, p_ast_exp p_exp, p_ast_stmt p_stmt_1);
void ast2ir_stmt_if_else_gen(p_ast2ir_info p_info, p_ir_basic_block p_while_start, p_ir_basic_block p_while_end_next, p_ast_exp p_exp, p_ast_stmt p_stmt_1, p_ast_stmt p_stmt_2);
void ast2ir_stmt_while_gen(p_ast2ir_info p_info, p_ast_exp p_exp, p_ast_stmt p_stmt_1);
void ast2ir_stmt_assign_gen(p_ast2ir_info p_info, p_ast_exp p_lval, p_ast_exp p_rval);
void ast2ir_stmt_block_gen(p_ast2ir_info p_info,
    p_ir_basic_block while_start, p_ir_basic_block while_end_next, p_ast_block p_block);
#endif
