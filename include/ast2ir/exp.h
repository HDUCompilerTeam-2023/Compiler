#ifndef __AST2IR_INSTR__
#define __AST2IR_INSTR__

#include <ast2ir/info_gen.h>

p_ir_operand ast2ir_exp_gen(p_ast2ir_info p_info, p_ast_exp p_exp);
p_ir_operand ast2ir_exp_cond_gen(p_ast2ir_info p_info, p_ir_basic_block p_true_block, p_ir_basic_block p_false_block, p_ast_exp p_exp);

#endif
