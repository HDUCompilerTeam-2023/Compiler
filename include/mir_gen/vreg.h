#ifndef __MIR_GEN_VREG__
#define __MIR_GEN_VREG__

#include <mir/vreg.h>

p_mir_vreg mir_vreg_gen(basic_type b_type, size_t ref_level);

void mir_vreg_set_bb_def(p_mir_vreg p_vreg, p_mir_basic_block p_bb);
void mir_vreg_set_instr_def(p_mir_vreg p_vreg, p_mir_instr p_instr);

void mir_vreg_drop(p_mir_vreg p_vreg);

#endif
