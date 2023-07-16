#include <stdio.h>
#include <util.h>
typedef enum arm_instr_type arm_instr_type;
typedef enum arm_cond_type arm_cond_type;

enum arm_instr_type {
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
    arm_mul,
    arm_div,

    arm_store,
    arm_load,

    arm_cmp,
    arm_cmn,
    arm_tst,
    arm_teq,

    arm_mov,
    arm_mvn,
    arm_movt,
    arm_movw,
    arm_int2float,
    arm_float2int,

    arm_b,
    arm_bl,
    arm_bx,
    arm_blx,
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
char *uint32_str(uint32_t intconst);

void arm_data_process_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t rs, size_t operand, bool s, size_t lsl_imme, bool if_imme);

void arm_load_gen(FILE *out_file, size_t rd, size_t rn, size_t offset, size_t lsl_imme, bool if_imme);
void arm_store_gen(FILE *out_file, size_t rd, size_t rn, size_t offset, size_t lsl_imme, bool if_imme);
void arm_compare_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t operand, size_t lsl_imme, bool if_imme);
void arm_mov16_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t imme16, bool s);
void arm_mov_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t operand, bool s, size_t lsl_imme, bool if_imme);
void arm_movcond_gen(FILE *out_file, arm_cond_type cond_type, size_t rd, size_t operand, size_t lsl_imme, bool if_imme);
void arm_mul_gen(FILE *out_file, size_t rd, size_t rm, size_t rs, bool s);
void arm_sdiv_gen(FILE *out_file, size_t rd, size_t rm, size_t rs);
void arm_swap_gen(FILE *out_file, size_t r1, size_t r2);

void arm_global_sym_declare_gen(FILE *out_file, char *p_sym, size_t size);
void arm_func_sym_declare_gen(FILE *out_file, char *p_func);

void arm_jump_label_gen(FILE *out_file, arm_instr_type type, arm_cond_type cond_type, char *label);
void arm_jump_reg_gen(FILE *out_file, arm_instr_type type, arm_cond_type cond_type, size_t r_target);

void arm_label_gen(FILE *out_file, char *name);

void arm_get_global_addr(FILE *out_file, size_t id, char *name);

void arm_word_gen(FILE *out_file, size_t imme32);
void arm_space_gen(FILE *out_file, size_t size);

void arm_push_gen(FILE *out_file, size_t *reg_id, size_t num);
void arm_pop_gen(FILE *out_file, size_t *reg_id, size_t num);

void arm_vmov_gen(FILE *out_file, size_t rd, size_t rs);
void arm_vset_flag(FILE *out_file, size_t r);
void arm_vdata_process_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t rs1, size_t rs2);
void arm_vneg_gen(FILE *out_file, size_t rd, size_t rs);
void arm_vcvt_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t rs);
void arm_vcompare_gen(FILE *out_file, arm_instr_type type, size_t rs1, size_t rs2);
void arm_vpush_gen(FILE *out_file, size_t *reg_id, size_t num);
void arm_vpop_gen(FILE *out_file, size_t *reg_id, size_t num);
void arm_vload_gen(FILE *out_file, size_t rd, size_t rs, size_t offset);
void arm_vstore_gen(FILE *out_file, size_t rd, size_t rs, size_t offset);
