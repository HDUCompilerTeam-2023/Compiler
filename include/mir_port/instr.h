#ifndef __MIR_PORT_INSTR__
#define __MIR_PORT_INSTR__
#include <mir.h>

p_mir_operand mir_instr_binary_get_src1(p_mir_instr p_instr);
p_mir_operand mir_instr_binary_get_src2(p_mir_instr p_instr);
p_mir_vreg mir_instr_binary_get_des(p_mir_instr p_instr);

p_mir_operand mir_instr_unary_get_src(p_mir_instr p_instr);
p_mir_vreg mir_instr_unary_get_des(p_mir_instr p_instr);

p_mir_operand mir_instr_ret_get_src(p_mir_instr p_instr);

p_mir_func mir_instr_call_get_func(p_mir_instr p_instr);
p_mir_vreg mir_instr_call_get_des(p_mir_instr p_instr);
p_mir_param_list mir_instr_call_get_param(p_mir_instr p_instr);

p_mir_operand mir_instr_load_get_array(p_mir_instr p_instr);
p_mir_operand mir_instr_array_get_offset(p_mir_instr p_instr);
p_mir_vreg mir_instr_array_get_des(p_mir_instr p_instr);

p_mir_operand mir_instr_array_assign_get_array(p_mir_instr p_instr);
p_mir_operand mir_instr_array_assign_get_offset(p_mir_instr p_instr);
p_mir_operand mir_instr_array_assign_get_src(p_mir_instr p_instr);

p_mir_basic_block mir_instr_br_get_target(p_mir_instr p_instr);

p_mir_operand mir_instr_condbr_get_cond(p_mir_instr p_instr);
p_mir_basic_block mir_instr_condbr_get_target_true(p_mir_instr p_instr);
p_mir_basic_block mir_instr_condbr_get_target_false(p_mir_instr p_instr);

mir_instr_type mir_instr_get_kind(p_mir_instr p_instr);
#endif
