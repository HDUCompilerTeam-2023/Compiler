#ifndef __LIR_GEN_ARM_STANDARD__
#define __LIR_GEN_ARM_STANDARD__
#include <symbol.h>
#include <util.h>
bool if_legal_rotate_imme12(I32CONST_t i32const);
bool if_legal_direct_imme12(I32CONST_t i32const);
bool if_legal_imme8_lsl2(I32CONST_t i32const);
bool if_in_r(p_symbol_type p_type);
bool if_caller_save_reg(size_t id);
#define imm_8_max 255
#define imm_12_max 4095
#define imm_16_max 65535

#define R_NUM 16
#define S_NUM 32
#define REG_NUM 48
#define FP 11
#define SP 13
#define LR 14
#define PC 15
#define TMP 14

#define temp_reg_num_r 4
#define temp_reg_num_s 16

#define caller_save_reg_num_r 5
#define callee_save_reg_num_r 8
#define caller_save_reg_num_s 16
#define callee_save_reg_num_s 16

extern size_t caller_save_reg_r[caller_save_reg_num_r];
extern size_t callee_save_reg_r[callee_save_reg_num_r];
extern size_t caller_save_reg_s[caller_save_reg_num_s];
extern size_t callee_save_reg_s[callee_save_reg_num_s];

extern size_t temp_reg_r[temp_reg_num_r];
extern size_t temp_reg_s[temp_reg_num_s];

#endif