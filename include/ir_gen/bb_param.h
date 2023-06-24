#ifndef __IR_GEN_BB_PARAM__
#define __IR_GEN_BB_PARAM__
#include <ir/bb_param.h>

p_ir_bb_phi_list ir_bb_phi_list_init(void);
p_ir_bb_phi_list ir_bb_phi_list_add(p_ir_bb_phi_list p_bb_phi_list, p_ir_vreg p_bb_phi);
void ir_bb_phi_list_drop(p_ir_bb_phi_list p_bb_phi_list);

p_ir_bb_param_list ir_bb_param_list_init(void);
p_ir_bb_param_list ir_bb_param_list_add(p_ir_bb_param_list p_bb_param_list, p_ir_operand p_bb_param);
void ir_bb_param_list_drop(p_ir_bb_param_list p_bb_param_list);
void copy_live(p_ir_bb_phi_list p_des, p_ir_bb_phi_list p_src);
void live_set_del(p_ir_bb_phi_list p_list, p_ir_vreg p_vreg);

#endif
