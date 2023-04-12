#ifndef __MIR_GEN_OPERAND__
#define __MIR_GEN_OPERAND__

#include <mir/operand.h>

p_mir_symbol mir_declared_sym_gen(p_symbol_sym p_sym);
p_mir_symbol mir_temp_sym_gen(p_mir_func p_func, p_symbol_type p_type);

p_mir_operand mir_operand_int_gen(int intconst);
p_mir_operand mir_operand_float_gen(float floatconst);
p_mir_operand mir_operand_sym_gen(p_mir_symbol p_mir_sym);

void mir_symbol_drop(p_mir_symbol p_sym);
void mir_operand_drop(p_mir_operand p_operand);
#endif