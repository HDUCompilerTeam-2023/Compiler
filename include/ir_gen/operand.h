#ifndef __IR_GEN_OPERAND__
#define __IR_GEN_OPERAND__

#include <ir/operand.h>
void ir_operand_reset_int(p_ir_operand p_operand, I32CONST_t intconst);
void ir_operand_reset_float(p_ir_operand p_operand, F32CONST_t floatconst);
void ir_operand_reset_str(p_ir_operand p_operand, p_symbol_str strconst);
void ir_operand_reset_void(p_ir_operand p_operand);
p_ir_operand ir_operand_int_gen(I32CONST_t intconst);
p_ir_operand ir_operand_float_gen(F32CONST_t floatconst);
p_ir_operand ir_operand_str_gen(p_symbol_str strconst);
p_ir_operand ir_operand_void_gen(void);


void ir_operand_reset_addr(p_ir_operand p_operand, p_symbol_var p_vmem, p_symbol_type p_type, I32CONST_t offset);
p_ir_operand ir_operand_addr_gen(p_symbol_var p_vmem, p_symbol_type p_type, I32CONST_t offset);


void ir_operand_reset_vreg(p_ir_operand p_operand, p_ir_vreg p_vreg);
p_ir_operand ir_operand_vreg_gen(p_ir_vreg p_vreg);

void ir_operand_reset_operand(p_ir_operand p_operand, p_ir_operand p_src);
p_ir_operand ir_operand_copy(p_ir_operand p_src);

basic_type ir_operand_get_basic_type(p_ir_operand p_operand);

void ir_operand_drop(p_ir_operand p_operand);
#endif
