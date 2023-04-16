#ifndef __MIR_GEN_OPERAND__
#define __MIR_GEN_OPERAND__

#include <mir/operand.h>
p_mir_operand mir_operand_int_gen(int intconst);
p_mir_operand mir_operand_float_gen(float floatconst);
p_mir_operand mir_operand_declared_sym_gen(p_symbol_sym p_mir_sym);
p_mir_operand mir_operand_temp_sym_gen(size_t id, p_symbol_type p_type);

p_symbol_type mir_operand_sym_type_gen(basic_type b_type);

void mir_operand_drop(p_mir_operand p_operand);

#endif