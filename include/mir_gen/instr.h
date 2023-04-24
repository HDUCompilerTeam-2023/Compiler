#ifndef __MIR_GEN_INSTR__
#define  __MIR_GEN_INSTR__
#include <mir/instr.h>

p_mir_instr mir_binary_instr_gen(mir_instr_type mir_type, p_mir_operand p_src1, p_mir_operand p_src2, p_mir_operand des);
p_mir_instr mir_unary_instr_gen(mir_instr_type mir_type, p_mir_operand p_src, p_mir_operand p_des);

p_mir_instr mir_call_instr_gen(p_mir_func p_func, p_mir_param_list p_param_list, p_mir_operand p_des);
p_mir_instr mir_array_instr_gen(p_mir_operand p_array, p_mir_operand p_offset, p_mir_operand p_des);
p_mir_instr mir_array_assign_instr_gen(p_mir_operand p_sym, p_mir_operand p_offset, p_mir_operand p_src);
p_mir_instr mir_ret_instr_gen(p_mir_operand p_src);
p_mir_instr mir_br_instr_gen(p_mir_basic_block p_current_basic_block, p_mir_basic_block p_target);
p_mir_instr mir_condbr_instr_gen(p_mir_basic_block p_current_basic_block, p_mir_operand p_cond, p_mir_basic_block p_target_true, p_mir_basic_block p_target_false);

p_mir_operand mir_instr_get_src1(p_mir_instr p_instr);
p_mir_operand mir_instr_get_src2(p_mir_instr p_instr);
p_mir_operand mir_instr_get_des(p_mir_instr p_instr);

void mir_instr_drop(p_mir_instr p_instr);
#endif