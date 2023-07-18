#ifndef __IR_GEN_INSTR__
#define __IR_GEN_INSTR__
#include <ir/instr.h>

p_ir_instr ir_binary_instr_gen(ir_binary_op op, p_ir_operand p_src1, p_ir_operand p_src2, p_ir_vreg des);
p_ir_instr ir_unary_instr_gen(ir_unary_op op, p_ir_operand p_src, p_ir_vreg p_des);

p_ir_instr ir_call_instr_gen(p_symbol_func p_func, p_ir_vreg p_des);
void ir_call_param_list_add(p_ir_instr p_instr, p_ir_operand p_param);

p_ir_instr ir_gep_instr_gen(p_ir_operand p_addr, p_ir_operand p_offset, p_ir_vreg p_des, bool is_element);
p_ir_instr ir_load_instr_gen(p_ir_operand p_addr, p_ir_vreg p_des, bool is_stack_ptr);
p_ir_instr ir_store_instr_gen(p_ir_operand p_addr, p_ir_operand p_src, bool is_stack_ptr);

void ir_instr_reset_binary(p_ir_instr p_instr, ir_binary_op op, p_ir_operand p_src1, p_ir_operand p_src2, p_ir_vreg p_des);
void ir_instr_reset_unary(p_ir_instr p_instr, ir_unary_op op, p_ir_operand p_src, p_ir_vreg p_des);
void ir_instr_reset_call(p_ir_instr p_instr, p_symbol_func p_func, p_ir_vreg p_des);
void ir_instr_reset_gep(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_operand p_offset, p_ir_vreg p_des, bool is_element);
void ir_instr_reset_load(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_vreg p_des, bool is_stack_ptr);
void ir_instr_reset_store(p_ir_instr p_instr, p_ir_operand p_addr, p_ir_operand p_src, bool is_stack_ptr);

void ir_instr_add_next(p_ir_instr p_next, p_ir_instr p_prev);
void ir_instr_add_prev(p_ir_instr p_prev, p_ir_instr p_next);

p_ir_operand ir_instr_get_src1(p_ir_instr p_instr);
p_ir_operand ir_instr_get_src2(p_ir_instr p_instr);
p_ir_vreg ir_instr_get_des(p_ir_instr p_instr);
p_ir_operand ir_instr_get_load_addr(p_ir_instr p_instr);
p_ir_operand ir_instr_get_store_addr(p_ir_instr p_instr);

void ir_set_load_instr_des(p_ir_instr p_load, p_ir_vreg p_des);
void ir_set_load_instr_addr(p_ir_instr p_load, p_ir_operand p_addr);
void ir_set_binary_instr_des(p_ir_instr p_binary, p_ir_vreg p_des);
void ir_set_binary_instr_src1(p_ir_instr p_load, p_ir_operand p_src1);
void ir_set_binary_instr_src2(p_ir_instr p_load, p_ir_operand p_src2);
void ir_set_unary_instr_des(p_ir_instr p_unary, p_ir_vreg p_des);
void ir_set_unary_instr_src(p_ir_instr p_unary, p_ir_operand p_src);
void ir_set_gep_instr_des(p_ir_instr p_gep, p_ir_vreg p_des);
void ir_set_gep_instr_addr(p_ir_instr p_gep, p_ir_operand p_addr);
void ir_set_gep_instr_offset(p_ir_instr p_gep, p_ir_operand p_offset);
void ir_set_call_instr_des(p_ir_instr p_call, p_ir_vreg p_des);
void ir_set_store_instr_src(p_ir_instr p_store, p_ir_operand p_src);
void ir_set_store_instr_offset(p_ir_instr p_store, p_ir_operand p_offset);

void ir_instr_del(p_ir_instr p_instr);
void ir_instr_drop(p_ir_instr p_instr);
#endif
