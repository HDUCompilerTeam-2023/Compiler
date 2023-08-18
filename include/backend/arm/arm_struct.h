#ifndef __BACKEND_ARM_ARM_STRUCT__
#define __BACKEND_ARM_ARM_STRUCT__
#include <symbol/var.h>
#include <util.h>
typedef struct arm_func arm_func, *p_arm_func;
typedef struct arm_block arm_block, *p_arm_block;
typedef struct arm_instr arm_instr, *p_arm_instr;
typedef struct arm_binary_instr arm_binary_instr, *p_arm_binary_instr;
typedef struct arm_mem_instr arm_mem_instr, *p_arm_mem_instr;
typedef struct arm_mov_instr arm_mov_instr, *p_arm_mov_instr;
typedef struct arm_mov32_instr arm_mov32_instr, *p_arm_mov32_instr;
typedef struct arm_cmp_instr arm_cmp_instr, *p_arm_cmp_instr;
typedef struct arm_jump_instr arm_jump_instr, *p_arm_jump_instr;
typedef struct arm_call_instr arm_call_instr, *p_arm_call_instr;
typedef struct arm_mul_instr arm_mul_instr, *p_arm_mul_instr;
typedef struct arm_sdiv_instr arm_sdiv_instr, *p_arm_sdiv_instr;
typedef struct arm_push_pop_instr arm_push_pop_instr, *p_arm_push_pop_instr;
typedef struct arm_vbinary_instr arm_vbinary_instr, *p_arm_vbinary_instr;
typedef struct arm_vmov_instr arm_vmov_instr, *p_arm_vmov_instr;
typedef struct arm_vmem_instr arm_vmem_instr, *p_arm_vmem_instr;
typedef struct arm_vcmp_instr arm_vcmp_instr, *p_arm_vcmp_instr;
typedef struct arm_vcvt_instr arm_vcvt_instr, *p_arm_vcvt_instr;
typedef struct arm_vneg_instr arm_vneg_instr, *p_arm_vneg_instr;
typedef struct arm_vpush_vpop_instr arm_vpush_vpop_instr, *p_arm_vpush_vpop_instr;

typedef enum arm_arch arm_arch;
typedef enum arm_mode arm_mode;
typedef enum arm_fpu arm_fpu;

typedef enum arm_instr_type arm_instr_type;
typedef enum arm_cond_type arm_cond_type;
typedef enum arm_binary_op arm_binary_op;
typedef enum arm_jump_op arm_jump_op;
typedef enum arm_mem_op arm_mem_op;
typedef enum arm_mov_op arm_mov_op;
typedef enum arm_cmp_op arm_cmp_op;
typedef enum arm_push_pop_op arm_push_pop_op;
typedef enum arm_vbinary_op arm_vbinary_op;
typedef enum arm_vmem_op arm_vmem_op;
typedef enum arm_vcvt_op arm_vcvt_op;
typedef enum arm_vcmp_op arm_vcmp_op;
typedef enum arm_vpush_vpop_op arm_vpush_vpop_op;

typedef uint32_t arm_imme;
typedef char *arm_label;
typedef size_t arm_reg;
typedef struct arm_operand arm_operand, *p_arm_operand;

struct arm_operand {
    bool if_imme;
    union {
        arm_imme imme;
        struct {
            arm_imme lsl_imme;
            arm_reg reg;
        };
    };
};
enum arm_arch {
    arm_v7ve,
};
enum arm_mode {
    arm,
    thumb,
};
enum arm_fpu {
    neon_vfpu,
};
enum arm_instr_type {
    arm_binary_type,
    arm_cmp_type,
    arm_mem_type,
    arm_jump_type,
    arm_call_type,
    arm_mov_type,
    arm_mov32_type,
    arm_mul_type,
    arm_sdiv_type,
    arm_push_pop_type,
    arm_vbinary_type,
    arm_vmov_type,
    arm_vmem_type,
    arm_vcmp_type,
    arm_vcvt_type,
    arm_vneg_type,
    arm_vpush_vpop_type,
};
enum arm_binary_op {
    arm_add,
    arm_sub,
    arm_rsb,
    arm_lsl,
    arm_lsr,
    arm_asr,
    arm_and,
    arm_eor,
    arm_orr,
    arm_orn,
};
enum arm_cmp_op {
    arm_cmp,
    arm_cmn,
    arm_tst,
    arm_teq,
};
enum arm_mem_op {
    arm_store,
    arm_load,
};
enum arm_mov_op {
    arm_mov,
    arm_mvn,
};
enum arm_push_pop_op {
    arm_push,
    arm_pop,
};
enum arm_vpush_vpop_op {
    arm_vpush,
    arm_vpop,
};
enum arm_vbinary_op {
    arm_vadd,
    arm_vsub,
    arm_vmul,
    arm_vdiv,
    arm_vand,
};
enum arm_vcvt_op {
    arm_int2float,
    arm_float2int,
};
enum arm_vmem_op {
    arm_vstore,
    arm_vload,
};
enum arm_vcmp_op {
    arm_vcmp,
    arm_vtst,
};
enum arm_cond_type {
    arm_eq,
    arm_ne,
    arm_ge,
    arm_gt,
    arm_le,
    arm_lt,
    arm_al,
};
enum arm_jump_op {
    arm_b,
    arm_bx,
    arm_bl,
    arm_blx,
};

struct arm_func {
    p_arm_block p_into_func_block;
    char *func_name;
    list_head node;
    list_head block_list;
};
struct arm_block {
    arm_label label;
    list_head instr_list;
    list_head node;
};

struct arm_binary_instr {
    arm_cond_type cond_type;
    arm_binary_op op;
    bool s;
    arm_reg rd;
    arm_reg rs1;
    p_arm_operand op2;
};
struct arm_jump_instr {
    arm_jump_op op;
    arm_cond_type cond_type;
    union {
        p_arm_block p_block_target;
        arm_reg reg_target;
    };
};

struct arm_call_instr {
    arm_label func_name;
};
struct arm_mov_instr {
    arm_cond_type type;
    arm_mov_op op;
    bool s;
    arm_reg rd;
    p_arm_operand operand;
};
struct arm_mov32_instr {
    arm_label label;
    arm_imme imme;
    arm_reg rd;
};

struct arm_mem_instr {
    arm_mem_op op;
    arm_reg rd;
    arm_reg base;
    p_arm_operand offset;
};

struct arm_cmp_instr {
    arm_cmp_op op;
    arm_reg rs1;
    p_arm_operand op2;
};

struct arm_mul_instr {
    bool s;
    arm_reg rd;
    arm_reg rs1;
    arm_reg rs2;
};

struct arm_sdiv_instr {
    arm_reg rd;
    arm_reg rs1;
    arm_reg rs2;
};

struct arm_push_pop_instr {
    arm_push_pop_op op;
    size_t num;
    arm_reg *regs;
};
struct arm_vmov_instr {
    arm_reg rd;
    arm_reg rs;
};
struct arm_vbinary_instr {
    arm_vbinary_op op;
    arm_reg rd;
    arm_reg rs1;
    arm_reg rs2;
};
struct arm_vneg_instr {
    arm_reg rd;
    arm_reg rs;
};
struct arm_vcvt_instr {
    arm_vcvt_op op;
    arm_reg rd;
    arm_reg rs;
};
struct arm_vcmp_instr {
    arm_vcmp_op op;
    arm_reg rs1;
    arm_reg rs2;
};
struct arm_vmem_instr {
    arm_vmem_op op;
    arm_reg rd;
    arm_reg base;
    arm_imme offset;
};
struct arm_vpush_vpop_instr {
    arm_vpush_vpop_op op;
    size_t num;
    arm_reg *regs;
};
struct arm_instr {
    arm_instr_type type;
    union {
        arm_binary_instr binary_instr;
        arm_mem_instr mem_instr;
        arm_jump_instr jump_instr;
        arm_call_instr call_instr;
        arm_mov_instr mov_instr;
        arm_cmp_instr cmp_instr;
        arm_mov32_instr mov32_instr;
        arm_mul_instr mul_instr;
        arm_sdiv_instr sdiv_instr;
        arm_push_pop_instr push_pop_instr;
        arm_vbinary_instr vbinary_instr;
        arm_vmov_instr vmov_instr;
        arm_vmem_instr vmem_instr;
        arm_vcmp_instr vcmp_instr;
        arm_vcvt_instr vcvt_instr;
        arm_vpush_vpop_instr vpush_vpop_instr;
        arm_vneg_instr vneg_instr;
    };
    list_head node;
};

extern size_t R_NUM, S_NUM;
#endif