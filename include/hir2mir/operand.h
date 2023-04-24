#ifndef __HIR2MIR_OPERAND__
#define __HIR2MIR_OPERAND__

#include <hir2mir/info_gen.h>
p_mir_operand hir2mir_operand_int_gen(p_hir2mir_info p_info, int intconst);
p_mir_operand hir2mir_operand_float_gen(p_hir2mir_info p_info, float floatconst);
p_mir_operand hir2mir_operand_void_gen(p_hir2mir_info p_info);

p_mir_operand hir2mir_operand_num_gen(p_hir2mir_info p_info, p_hir_exp p_exp);
p_mir_operand hir2mir_operand_declared_sym_gen(p_hir2mir_info p_info, p_symbol_sym p_sym);
p_mir_operand hir2mir_operand_temp_sym_array_gen(p_hir2mir_info p_info, basic_type b_type);
p_mir_operand hir2mir_operand_temp_sym_basic_gen(p_hir2mir_info p_info, basic_type b_type);

#endif