#ifndef __AST_GEN_EXP__
#define __AST_GEN_EXP__

#include <ast/exp.h>

void ast_exp_ptr_check_lval(p_ast_exp p_exp);
p_ast_exp ast_exp_ptr_check_const(p_ast_exp p_exp);
p_ast_exp ast_exp_ptr_to_val_check_basic(p_ast_exp p_exp);
p_ast_exp ast_exp_ptr_to_val(p_ast_exp p_exp);

p_ast_exp ast_exp_cov_gen(p_ast_exp p_exp, basic_type b_type);
p_ast_exp ast_exp_to_cond(p_ast_exp p_exp);
p_ast_exp ast_exp_use_gen(p_ast_exp p_used_exp);
p_ast_exp ast_exp_binary_gen(ast_exp_binary_op op, p_ast_exp p_src_1, p_ast_exp p_src_2);
p_ast_exp ast_exp_relational_gen(ast_exp_relational_op op, p_ast_exp p_rsrc_1, p_ast_exp p_rsrc_2);
p_ast_exp ast_exp_unary_gen(ast_exp_unary_op op, p_ast_exp p_src);
p_ast_exp ast_exp_logic_gen(ast_exp_logic_op l_op, p_ast_exp p_bool_1, p_ast_exp p_bool_2);
p_ast_exp ast_exp_ulogic_gen(ast_exp_ulogic_op ul_op, p_ast_exp p_bool);
p_ast_exp ast_exp_call_gen(p_symbol_func p_func, p_ast_param_list p_param_list);
p_ast_exp ast_exp_ptr_gen(p_symbol_var p_var);
p_ast_exp ast_exp_gep_gen(p_ast_exp p_val, p_ast_exp p_offset, bool is_element);
p_ast_exp ast_exp_load_gen(p_ast_exp p_ptr);
p_ast_exp ast_exp_int_gen(I32CONST_t num);
p_ast_exp ast_exp_float_gen(F32CONST_t num);
p_ast_exp ast_exp_str_gen(p_symbol_str p_str);
void ast_exp_drop(p_ast_exp p_exp);

#endif
