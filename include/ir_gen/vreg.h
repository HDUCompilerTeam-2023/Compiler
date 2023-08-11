#ifndef __IR_GEN_VREG__
#define __IR_GEN_VREG__

#include <ir/vreg.h>

p_ir_vreg ir_vreg_gen(p_symbol_type p_type);

p_ir_vreg ir_vreg_copy(p_ir_vreg p_vreg);
void ir_vreg_del(p_ir_vreg p_vreg);
void ir_vreg_drop(p_ir_vreg p_vreg);

p_ir_vreg_list ir_vreg_list_init(void);
p_ir_vreg_list ir_vreg_list_add(p_ir_vreg_list p_bb_phi_list, p_ir_vreg p_bb_phi);
void ir_vreg_list_drop(p_ir_vreg_list p_bb_phi_list);
p_ir_vreg_list_node ir_vreg_list_node_gen(p_ir_vreg p_vreg);
void ir_vreg_list_node_drop(p_ir_vreg_list_node p_vreg_list_node);
void ir_vreg_list_node_set_vreg(p_ir_vreg_list_node p_vreg_list_node, p_ir_vreg p_vreg);

void copy_vreg_list(p_ir_vreg_list p_des, p_ir_vreg_list p_src);
bool if_in_vreg_list(p_ir_vreg_list p_list, p_ir_vreg p_vreg);
void ir_vreg_list_del(p_ir_vreg_list p_list, p_ir_vreg p_vreg);
void ir_vreg_list_clear(p_ir_vreg_list p_list);
#endif
