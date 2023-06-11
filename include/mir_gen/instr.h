#ifndef __MIR_GEN_INSTR__
#define __MIR_GEN_INSTR__
#include <mir/instr.h>

p_mir_instr mir_binary_instr_gen(mir_binary_op op, p_mir_operand p_src1, p_mir_operand p_src2, p_mir_vreg des);
p_mir_instr mir_unary_instr_gen(mir_unary_op op, p_mir_operand p_src, p_mir_vreg p_des);

p_mir_instr mir_call_instr_gen(p_symbol_func p_func, p_mir_param_list p_param_list, p_mir_vreg p_des);

p_mir_instr mir_alloca_instr_gen(p_symbol_var p_vmem, p_mir_vreg p_des);
p_mir_instr mir_gep_instr_gen(p_mir_operand p_addr, p_mir_operand p_offset, p_mir_vreg p_des, bool is_element);
p_mir_instr mir_load_instr_gen(p_mir_operand p_addr, p_mir_operand p_offset, p_mir_vreg p_des);
p_mir_instr mir_store_instr_gen(p_mir_operand p_addr, p_mir_operand p_offset, p_mir_operand p_src);

p_mir_operand mir_instr_get_src1(p_mir_instr p_instr);
p_mir_operand mir_instr_get_src2(p_mir_instr p_instr);
p_mir_vreg mir_instr_get_des(p_mir_instr p_instr);
p_mir_operand mir_instr_get_load_addr(p_mir_instr p_instr);
p_mir_operand mir_instr_get_store_addr(p_mir_instr p_instr);

void mir_instr_drop(p_mir_instr p_instr);
#endif
