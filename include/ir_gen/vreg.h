#ifndef __IR_GEN_VREG__
#define __IR_GEN_VREG__

#include <ir/vreg.h>

p_ir_vreg ir_vreg_gen(p_symbol_type p_type);

void ir_vreg_set_bb_def(p_ir_vreg p_vreg, p_ir_basic_block p_bb);
void ir_vreg_set_instr_def(p_ir_vreg p_vreg, p_ir_instr p_instr);

p_ir_vreg ir_vreg_copy(p_ir_vreg p_vreg);
void ir_vreg_drop(p_ir_vreg p_vreg);

#endif
