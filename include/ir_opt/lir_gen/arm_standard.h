#ifndef __LIR_GEN_ARM_STANDARD__
#define __LIR_GEN_ARM_STANDARD__
#include <util.h>
bool if_legal_rotate_imme12(I32CONST_t i32const);
bool if_legal_direct_imme12(I32CONST_t i32const);
bool if_legal_direct_imme8(I32CONST_t i32const);

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
#endif