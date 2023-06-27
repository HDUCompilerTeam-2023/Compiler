#ifndef __IR_GEN_OPERAND__
#define __IR_GEN_OPERAND__

#include <ir/operand.h>
p_ir_operand ir_operand_int_gen(I32CONST_t intconst);
p_ir_operand ir_operand_float_gen(F32CONST_t floatconst);
p_ir_operand ir_operand_str_gen(p_symbol_str strconst);
p_ir_operand ir_operand_void_gen(void);

p_ir_operand ir_operand_addr_gen(p_symbol_var p_vmem);

p_ir_operand ir_operand_vreg_gen(p_ir_vreg p_vreg);

basic_type ir_operand_get_basic_type(p_ir_operand p_operand);

void ir_operand_drop(p_ir_operand p_operand);
#endif
