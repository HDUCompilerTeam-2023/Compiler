#ifndef __MIR_PRINT_INSTR__
#define __MIR_PRINT_INSTR__
#include <mir.h>
void mir_instr_print(p_mir_instr p_instr);

void mir_binary_instr_print(p_mir_binary_instr p_instr);
void mir_unary_instr_print(p_mir_unary_instr p_instr);
void mir_call_instr_print(p_mir_call_instr p_instr);
void mir_ret_instr_print(p_mir_ret_instr p_instr);
void mir_br_instr_print(p_mir_br_instr p_instr);
void mir_condbr_instr_print(p_mir_condbr_instr p_instr);
void mir_alloca_instr_print(p_mir_alloca_instr p_instr);
void mir_load_instr_print(p_mir_load_instr p_instr);
void mir_store_instr_print(p_mir_store_instr p_instr);

#endif
