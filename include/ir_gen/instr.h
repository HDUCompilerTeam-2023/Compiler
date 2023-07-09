#ifndef __IR_GEN_INSTR__
#define __IR_GEN_INSTR__
#include <ir/instr.h>

p_ir_instr ir_binary_instr_gen(ir_binary_op op, p_ir_operand p_src1, p_ir_operand p_src2, p_ir_vreg des);
p_ir_instr ir_unary_instr_gen(ir_unary_op op, p_ir_operand p_src, p_ir_vreg p_des);

p_ir_instr ir_call_instr_gen(p_symbol_func p_func, p_ir_param_list p_param_list, p_ir_vreg p_des);

p_ir_instr ir_gep_instr_gen(p_ir_operand p_addr, p_ir_operand p_offset, p_ir_vreg p_des, bool is_element);
p_ir_instr ir_load_instr_gen(p_ir_operand p_addr, p_ir_vreg p_des, bool is_stack_ptr);
p_ir_instr ir_store_instr_gen(p_ir_operand p_addr, p_ir_operand p_src, bool is_stack_ptr);

void ir_instr_reset_binary(p_ir_instr p_instr, ir_binary_op op, p_ir_operand p_src1, p_ir_operand p_src2, p_ir_vreg p_des);
void ir_instr_reset_unary(p_ir_instr p_instr, ir_unary_op op, p_ir_operand p_src, p_ir_vreg p_des);
void ir_instr_reset_call(p_ir_instr p_instr, p_symbol_func p_func, p_ir_param_list p_param_list, p_ir_vreg p_des);
void ir_instr_reset_gep(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_operand p_offset, p_ir_vreg p_des, bool is_element);
void ir_instr_reset_load(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_vreg p_des, bool is_stack_ptr);
void ir_instr_reset_store(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_operand p_src, bool is_stack_ptr);

p_ir_operand ir_instr_get_src1(p_ir_instr p_instr);
p_ir_operand ir_instr_get_src2(p_ir_instr p_instr);
p_ir_vreg ir_instr_get_des(p_ir_instr p_instr);
p_ir_operand ir_instr_get_load_addr(p_ir_instr p_instr);
p_ir_operand ir_instr_get_store_addr(p_ir_instr p_instr);

void ir_instr_drop(p_ir_instr p_instr);
#endif
