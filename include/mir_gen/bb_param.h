#ifndef __MIR_GEN_BB_PARAM__
#define __MIR_GEN_BB_PARAM__
#include <mir/bb_param.h>

p_mir_bb_phi_list mir_bb_phi_list_init(void);
p_mir_bb_phi_list mir_bb_phi_list_add(p_mir_bb_phi_list p_bb_phi_list, p_mir_vreg p_bb_phi);
void mir_bb_phi_list_drop(p_mir_bb_phi_list p_bb_phi_list);

p_mir_bb_param_list mir_bb_param_list_init(void);
p_mir_bb_param_list mir_bb_param_list_add(p_mir_bb_param_list p_bb_param_list, p_mir_operand p_bb_param);
void mir_bb_param_list_drop(p_mir_bb_param_list p_bb_param_list);

#endif
