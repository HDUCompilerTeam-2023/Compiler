#ifndef __HIR_GEN_EXP__
#define __HIR_GEN_EXP__

#include <hir/exp.h>

void hir_exp_ptr_check_lval(p_hir_exp p_exp);
p_hir_exp hir_exp_ptr_to_val_check_basic(p_hir_exp p_exp);
p_hir_exp hir_exp_ptr_to_val(p_hir_exp p_exp);

p_hir_exp hir_exp_use_gen(p_hir_exp p_used_exp);
p_hir_exp hir_exp_binary_gen(hir_exp_binary_op op, p_hir_exp p_src_1, p_hir_exp p_src_2);
p_hir_exp hir_exp_relational_gen(hir_exp_binary_op op, p_hir_exp p_src_1, p_hir_exp p_src_2);
p_hir_exp hir_exp_unary_gen(hir_exp_unary_op op, p_hir_exp p_src);
p_hir_exp hir_exp_logic_gen(hir_exp_logic_op l_op, p_hir_exp p_bool_1, p_hir_exp p_bool_2);
p_hir_exp hir_exp_ulogic_gen(hir_exp_ulogic_op ul_op, p_hir_exp p_bool);
p_hir_exp hir_exp_call_gen(p_symbol_func p_func, p_hir_param_list p_param_list);
p_hir_exp hir_exp_ptr_gen(p_symbol_var p_var);
p_hir_exp hir_exp_gep_gen(p_hir_exp p_val, p_hir_exp p_offset, bool is_element);
p_hir_exp hir_exp_load_gen(p_hir_exp p_ptr);
p_hir_exp hir_exp_int_gen(INTCONST_t num);
p_hir_exp hir_exp_float_gen(FLOATCONST_t num);
p_hir_exp hir_exp_str_gen(p_symbol_str p_str);
void hir_exp_drop(p_hir_exp p_exp);

#endif
