#ifndef __HIR2MIR_OPERAND__
#define __HIR2MIR_OPERAND__

#include <hir2mir/info_gen.h>
p_mir_temp_sym hir2mir_temp_sym_pointer_gen(basic_type b_type, p_mir_func p_func);
p_mir_temp_sym hir2mir_temp_sym_basic_gen(basic_type b_type, p_mir_func p_func);

p_mir_operand hir2mir_operand_temp_sym_array_gen(p_hir2mir_info p_info, basic_type b_type);
p_mir_operand hir2mir_operand_temp_sym_basic_gen(p_hir2mir_info p_info, basic_type b_type);

#endif