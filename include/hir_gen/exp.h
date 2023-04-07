#ifndef __HIR_GEN_EXP__
#define __HIR_GEN_EXP__

#include <hir/exp.h>

p_hir_exp hir_exp_assign_gen(p_hir_exp lval, p_hir_exp rval);
p_hir_exp hir_exp_exec_gen(hir_exp_op op, p_hir_exp p_src_1, p_hir_exp p_src_2);
p_hir_exp hir_exp_lexec_gen(hir_exp_op op, p_hir_exp p_src_1, p_hir_exp p_src_2);
p_hir_exp hir_exp_uexec_gen(hir_exp_op op, p_hir_exp p_src_1);
p_hir_exp hir_exp_call_gen(p_symbol_sym, p_hir_param_list p_param_list);
p_hir_exp hir_exp_id_gen(p_symbol_sym p_sym);
p_hir_exp hir_exp_arr_gen(p_hir_exp p_arrary, p_hir_exp p_index);
p_hir_exp hir_exp_int_gen(INTCONST_t num);
p_hir_exp hir_exp_float_gen(FLOATCONST_t num);
void hir_exp_drop(p_hir_exp p_exp);

#endif