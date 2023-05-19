#ifndef __MIR_GEN_OPERAND__
#define __MIR_GEN_OPERAND__

#include <mir/operand.h>
p_mir_operand mir_operand_int_gen(INTCONST_t intconst);
p_mir_operand mir_operand_float_gen(FLOATCONST_t floatconst);
p_mir_operand mir_operand_str_gen(p_symbol_str strconst);
p_mir_operand mir_operand_void_gen(void);

p_mir_operand mir_operand_addr_gen(p_mir_vmem p_global_vmem);

p_mir_operand mir_operand_vreg_gen(p_mir_vreg p_vreg);

basic_type mir_operand_get_basic_type(p_mir_operand p_operand);

void mir_operand_drop(p_mir_operand p_operand);
#endif