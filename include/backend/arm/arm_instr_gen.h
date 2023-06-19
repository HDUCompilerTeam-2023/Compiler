
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

void arm_data_process_gen(char *asm_code, arm_instr_type type, size_t rd, size_t rs, size_t operand, bool s, size_t lsl_imme, bool if_imme);

void arm_load_gen(char *asm_code, arm_instr_type type, size_t rd, size_t rn, size_t offset, size_t lsl_imme, bool if_imme);
void arm_store_gen(char *asm_code, arm_instr_type type, size_t rd, size_t rn, size_t offset, size_t lsl_imme, bool if_imme);
void arm_compare_gen(char *asm_code, arm_instr_type type, size_t rd, size_t operand, size_t lsl_imme, bool if_imme);
void arm_mov16_gen(char *asm_code, arm_instr_type type, size_t rd, size_t imme16, bool s);
void arm_mov_gen(char *asm_code, arm_instr_type type, size_t rd, size_t operand, bool s, size_t lsl_imme, bool if_imme);
void arm_movcond_gen(char *asm_code, arm_cond_type cond_type, size_t rd, size_t operand, size_t lsl_imme, bool if_imme);
void arm_mul_gen(char *asm_code, size_t rd, size_t rm, size_t rs, bool s);
void arm_sdiv_gen(char *asm_code, size_t rd, size_t rm, size_t rs);
void arm_swap_gen(char *asm_code, size_t r1, size_t r2);

void arm_global_sym_declare_gen(char *asm_code, char *p_sym, size_t size);
void arm_func_sym_declare_gen(char *asm_code, char *p_func);

void arm_jump_label_gen(char *asm_code, arm_instr_type type, arm_cond_type cond_type, char *label);
void arm_jump_reg_gen(char *asm_code, arm_instr_type type, arm_cond_type cond_type, size_t r_target);

void arm_label_gen(char *asm_code, char *name);

void arm_get_global_addr(char *asm_code, size_t id, char *name);

void arm_word_gen(char *asm_code, size_t imme32);
void arm_space_gen(char *asm_code, size_t size);

void arm_push_gen(char *asm_code, size_t *reg_id, size_t num);
void arm_pop_gen(char *asm_code, size_t *reg_id, size_t num);

void arm_get_float_label_val(char *asm_code, size_t rd, char *func_name, size_t len);
void arm_vmov_gen(char *asm_code, size_t rd, size_t rs);
void arm_float_code_gen(char *asm_code, char *func_name, char *extra_code);