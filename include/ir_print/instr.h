#ifndef __IR_PRINT_INSTR__
#define __IR_PRINT_INSTR__
#include <ir.h>
void ir_instr_print(p_ir_instr p_instr);

void ir_binary_instr_print(p_ir_binary_instr p_instr);
void ir_unary_instr_print(p_ir_unary_instr p_instr);
void ir_call_instr_print(p_ir_call_instr p_instr);
void ir_ret_instr_print(p_ir_ret_instr p_instr);
void ir_br_instr_print(p_ir_br_instr p_instr);
void ir_condbr_instr_print(p_ir_condbr_instr p_instr);
void ir_alloca_instr_print(p_ir_alloca_instr p_instr);
void ir_gep_instr_print(p_ir_gep_instr p_instr);
void ir_load_instr_print(p_ir_load_instr p_instr);
void ir_store_instr_print(p_ir_store_instr p_instr);

#endif
