#ifndef __BACKEND_ARM_STRUCT_GEN__
#define __BACKEND_ARM_STRUCT_GEN__
#include <backend/arm/arm_struct.h>
arm_label arm_label_gen(char *label);
p_arm_func arm_func_gen(arm_label label);
p_arm_block arm_block_gen(arm_label label);
void arm_func_add_block_tail(p_arm_func p_func, p_arm_block p_block);
void arm_block_add_instr_tail(p_arm_block p_block, p_arm_instr p_instr);

p_arm_operand arm_operand_imme_gen(arm_imme imme);
p_arm_operand arm_operand_reg_gen(arm_reg p_reg);

p_arm_instr arm_binary_instr_gen(arm_binary_op op, arm_reg rd, arm_reg rs1, p_arm_operand op2);
p_arm_instr arm_cmp_instr_gen(arm_cmp_op op, arm_reg rs1, p_arm_operand op2);
void arm_block_set_br1(p_arm_block p_a_block, p_arm_instr p_instr);
void arm_block_set_br2(p_arm_block p_a_block, p_arm_instr p_instr);
void arm_block_add_prev(p_arm_block p_source, p_arm_instr p_instr);
void arm_block_del_prev(p_arm_instr p_jump);
void arm_jump_instr_reset_target(p_arm_instr p_jump, p_arm_block p_target);
p_arm_instr arm_jump_instr_gen(arm_jump_op op, arm_cond_type cond_type, p_arm_block p_source_block, p_arm_block block);
p_arm_instr arm_call_instr_gen(arm_label label);
p_arm_instr arm_mem_instr_gen(arm_mem_op op, arm_reg rd, arm_reg base, p_arm_operand offset);
p_arm_instr arm_mov_instr_gen(arm_mov_op op, arm_cond_type cond, arm_reg rd, p_arm_operand op1);
p_arm_instr arm_mov32_instr_gen(arm_reg rd, arm_label label, arm_imme imme);
p_arm_instr arm_mul_instr_gen(arm_reg rd, arm_reg rs1, arm_reg rs2);
p_arm_instr arm_sdiv_instr_gen(arm_reg rd, arm_reg rs1, arm_reg rs2);
p_arm_instr arm_push_pop_instr_gen(arm_push_pop_op op);
void arm_push_pop_instr_init(p_arm_push_pop_instr p_instr, size_t *id, size_t num);
void arm_push_pop_instr_add(p_arm_push_pop_instr p_instr, size_t reg);
p_arm_instr arm_vbinary_instr_gen(arm_vbinary_op op, arm_reg rd, arm_reg rs1, arm_reg rs2);
p_arm_instr arm_vmov_instr_gen(arm_reg rd, arm_reg rs);
p_arm_instr arm_vcmp_instr_gen(arm_vcmp_op op, arm_reg rs1, arm_reg rs2);
p_arm_instr arm_vcvt_instr_gen(arm_vcvt_op op, arm_reg rd, arm_reg rs);
p_arm_instr arm_vmem_instr_gen(arm_vmem_op op, arm_reg rd, arm_reg base, arm_imme offset);
p_arm_instr arm_vneg_instr_gen(arm_reg rd, arm_reg rs);
p_arm_instr arm_vpush_vpop_instr_gen(arm_vpush_vpop_op op);
void arm_vpush_vpop_instr_add(p_arm_vpush_vpop_instr p_instr, size_t reg);
void arm_vpush_vpop_instr_init(p_arm_vpush_vpop_instr p_instr, size_t *id, size_t num);

void arm_instr_drop(p_arm_instr p_instr);
void arm_block_drop(p_arm_block p_block);
void arm_func_drop(p_arm_func p_func);

arm_label arm_block_label_gen(char *func, size_t id);
arm_cond_type get_opposite_type(arm_cond_type type);
bool if_float_reg(arm_reg reg);

p_arm_instr arm_instr_copy(p_arm_instr p_src);
#endif