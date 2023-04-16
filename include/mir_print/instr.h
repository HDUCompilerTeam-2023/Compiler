#ifndef __MIR_PRINT_INSTR__
#define __MIR_PRINT_INSTR__
#include <mir.h>
void mir_instr_print(p_mir_instr p_instr);

void mir_binary_instr_print(mir_instr_type instr_type, p_mir_binary_instr p_instr);
void mir_unary_instr_print(mir_instr_type instr_type, p_mir_unary_instr p_instr);
void mir_call_instr_print(p_mir_call_instr p_instr);
void mir_ret_instr_print(p_mir_ret_instr p_instr);
void mir_br_instr_print(p_mir_br_instr p_instr);
void mir_condbr_instr_print(p_mir_condbr_instr p_instr);
void mir_array_instr_print(p_mir_array_instr p_instr);
void mir_array_assign_instr_print(p_mir_array_assign_instr p_instr);

#endif
