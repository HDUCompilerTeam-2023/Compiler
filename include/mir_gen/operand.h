#ifndef __MIR_GEN_OPERAND__
#define __MIR_GEN_OPERAND__

#include <mir/operand.h>

p_mir_operand mir_operand_int_gen(int intconst);
p_mir_operand mir_operand_float_gen(float floatconst);
p_mir_operand mir_operand_sym_gen(p_mir_symbol p_mir_sym);

void mir_operand_drop(p_mir_operand p_operand);

#endif