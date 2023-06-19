
#include <backend/arm/arm_instr_gen.h>

static char regs[48][4] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc", "s0", "s1", "s2", "s3", "s4", "s5", "s16",
    "s7", "s8", "s9", "s10", "s11", "s12", "s13",
    "s14", "s15", "s16", "s17", "s18", "s19", "s20",
    "s21", "s22", "s23", "s24", "s25", "s26", "s27",
    "s28", "s29", "s30", "s31" };

static const size_t R_NUM = 16;
// static const size_t S_NUM = 32;

char *uint32_str(uint32_t intconst) {
    if (!intconst) {
        char *str = malloc(sizeof(*str) * 2);
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    uint32_t tmp = intconst;
    size_t len = 0;
    while (tmp) {
        tmp /= 10;
        len++;
    }
    char *str = malloc((len + 1) * sizeof(*str));
    str[len] = '\0';
    while (intconst) {
        str[--len] = intconst % 10 + 48;
        intconst /= 10;
    }
    return str;
}

static inline void uint32_str_cat(char *prefix, uint32_t intconst) {
    char *str = uint32_str(intconst);
    strcat(prefix, str);
    free(str);
}

static inline void operand2str(char *asm_code, size_t operand, size_t lsl_imme, bool if_imme) {
    if (if_imme) {
        if (lsl_imme)
            operand <<= lsl_imme;
        strcat(asm_code, "#");
        uint32_str_cat(asm_code, operand);
        return;
    }
    strcat(asm_code, regs[operand]);
    if (lsl_imme) {
        strcat(asm_code, ", lsl #");
        uint32_str_cat(asm_code, lsl_imme);
    }
}

static inline void arm_cond_type_gen(char *asm_code, arm_cond_type cond_type) {
    switch (cond_type) {
    case arm_eq:
        strcat(asm_code, "eq ");
        break;
    case arm_ne:
        strcat(asm_code, "ne ");
        break;
    case arm_ge:
        strcat(asm_code, "ge ");
        break;
    case arm_gt:
        strcat(asm_code, "gt ");
        break;
    case arm_le:
        strcat(asm_code, "le ");
        break;
    case arm_lt:
        strcat(asm_code, "lt ");
        break;
    case arm_al:
        strcat(asm_code, " ");
        break;
    }
}

void arm_data_process_gen(char *asm_code, arm_instr_type type, size_t rd, size_t rs, size_t operand, bool s, size_t lsl_imme, bool if_imme) {
    switch (type) {
    case arm_add:
        strcat(asm_code, "   add");
        break;
    case arm_sub:
        strcat(asm_code, "   sub");
        break;
    case arm_rsb:
        strcat(asm_code, "   rsb");
        break;
    case arm_lsl:
        strcat(asm_code, "   lsl");
        break;
    case arm_lsr:
        strcat(asm_code, "   lsr");
        break;
    case arm_asr:
        strcat(asm_code, "   asr");
        break;
    case arm_and:
        strcat(asm_code, "   and");
        break;
    case arm_eor:
        strcat(asm_code, "   eor");
        break;
    case arm_orr:
        strcat(asm_code, "   orr");
        break;
    case arm_orn:
        strcat(asm_code, "   orn");
        break;
    default:
        assert(0);
    }
    if (s)
        strcat(asm_code, "s");
    strcat(asm_code, " ");
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", ");
    strcat(asm_code, regs[rs]);
    strcat(asm_code, ", ");
    operand2str(asm_code, operand, lsl_imme, if_imme);
    strcat(asm_code, "\n");
}

// if_imme 表示 operand 是否是立即数
void arm_load_gen(char *asm_code, arm_instr_type type, size_t rd, size_t rn, size_t offset, size_t lsl_imme, bool if_imme) {
    strcat(asm_code, "   ldr ");
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", [");
    strcat(asm_code, regs[rn]);
    if (!(if_imme && !offset)) { // 偏移不为0
        strcat(asm_code, ", ");
        operand2str(asm_code, offset, lsl_imme, if_imme);
    }
    strcat(asm_code, "]\n");
}

void arm_store_gen(char *asm_code, arm_instr_type type, size_t rd, size_t rn, size_t offset, size_t lsl_imme, bool if_imme) {
    strcat(asm_code, "   str ");
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", [");
    strcat(asm_code, regs[rn]);
    if (!(if_imme && !offset)) { // 偏移不为0
        strcat(asm_code, ", ");
        operand2str(asm_code, offset, lsl_imme, if_imme);
    }
    strcat(asm_code, "]\n");
}

void arm_compare_gen(char *asm_code, arm_instr_type type, size_t rd, size_t operand, size_t lsl_imme, bool if_imme) {
    switch (type) {
    case arm_cmp:
        strcat(asm_code, "   cmp ");
        break;
    case arm_cmn:
        strcat(asm_code, "   cmn ");
        break;
    case arm_tst:
        strcat(asm_code, "   tst ");
        break;
    case arm_teq:
        strcat(asm_code, "   teq ");
        break;
    default:
        assert(0);
    }
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", ");
    operand2str(asm_code, operand, lsl_imme, if_imme);
    strcat(asm_code, "\n");
}

void arm_mov16_gen(char *asm_code, arm_instr_type type, size_t rd, size_t imme16, bool s) {
    switch (type) {
    case arm_movw:
        strcat(asm_code, "   movw ");
        break;
    case arm_movt:
        strcat(asm_code, "   movt ");
        break;
    default:
        assert(0);
    }
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", #");
    uint32_str_cat(asm_code, imme16);
    strcat(asm_code, "\n");
    if (s)
        arm_compare_gen(asm_code, arm_cmp, rd, 0, 0, true);
}

void arm_mov_gen(char *asm_code, arm_instr_type type, size_t rd, size_t operand, bool s, size_t lsl_imme, bool if_imme) {
    switch (type) {
    case arm_mov:
        strcat(asm_code, "   mov");
        break;
    case arm_mvn:
        strcat(asm_code, "   mvn");
        break;
    default:
        assert(0);
    }
    if (s)
        strcat(asm_code, "s");
    strcat(asm_code, " ");
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", ");
    operand2str(asm_code, operand, lsl_imme, if_imme);
    strcat(asm_code, "\n");
}

void arm_movcond_gen(char *asm_code, arm_cond_type cond_type, size_t rd, size_t operand, size_t lsl_imme, bool if_imme) {
    strcat(asm_code, "   mov");
    arm_cond_type_gen(asm_code, cond_type);
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", ");
    operand2str(asm_code, operand, lsl_imme, if_imme);
    strcat(asm_code, "\n");
}

void arm_mul_gen(char *asm_code, size_t rd, size_t rm, size_t rs, bool s) {
    strcat(asm_code, "   mul");
    if (s)
        strcat(asm_code, "s");
    strcat(asm_code, " ");
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", ");
    strcat(asm_code, regs[rm]);
    strcat(asm_code, ", ");
    strcat(asm_code, regs[rs]);
    strcat(asm_code, "\n");
}

void arm_sdiv_gen(char *asm_code, size_t rd, size_t rm, size_t rs) {
    strcat(asm_code, "  sdiv ");
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", ");
    strcat(asm_code, regs[rm]);
    strcat(asm_code, ", ");
    strcat(asm_code, regs[rs]);
    strcat(asm_code, "\n");
}

void arm_label_gen(char *asm_code, char *name) {
    strcat(asm_code, name);
    strcat(asm_code, ":\n");
}

void arm_jump_reg_gen(char *asm_code, arm_instr_type type, arm_cond_type cond_type, size_t r_target) {
    switch (type) {
    case arm_bx:
        strcat(asm_code, "   bx");
        break;
    case arm_blx:
        strcat(asm_code, "   blx");
        break;
    default:
        assert(0);
        break;
    }
    arm_cond_type_gen(asm_code, cond_type);
    strcat(asm_code, regs[r_target]);
    strcat(asm_code, "\n");
}

void arm_jump_label_gen(char *asm_code, arm_instr_type type, arm_cond_type cond_type, char *label) {
    switch (type) {
    case arm_b:
        strcat(asm_code, "   b");
        break;
    case arm_bl:
        strcat(asm_code, "   bl");
        break;
    default:
        assert(0);
    }
    arm_cond_type_gen(asm_code, cond_type);
    strcat(asm_code, label);
    strcat(asm_code, "\n");
}

void arm_swap_gen(char *asm_code, size_t r1, size_t r2) {
    if (r1 == r2) return;
    arm_data_process_gen(asm_code, arm_eor, r1, r1, r2, false, 0, false);
    arm_data_process_gen(asm_code, arm_eor, r2, r1, r2, false, 0, false);
    arm_data_process_gen(asm_code, arm_eor, r1, r1, r2, false, 0, false);
}

void arm_word_gen(char *asm_code, size_t imme32) {
    strcat(asm_code, "   .word ");
    uint32_str_cat(asm_code, imme32);
    strcat(asm_code, "\n");
}

void arm_space_gen(char *asm_code, size_t size) {
    strcat(asm_code, "   .space ");
    uint32_str_cat(asm_code, size);
    strcat(asm_code, "\n");
}

void arm_global_sym_declare_gen(char *asm_code, char *p_sym, size_t size) {
    strcat(asm_code, "\n");
    strcat(asm_code, ".global ");
    strcat(asm_code, p_sym);
    strcat(asm_code, "\n");
    strcat(asm_code, "   .type ");
    strcat(asm_code, p_sym);
    strcat(asm_code, ", %object\n");
    strcat(asm_code, "   .data\n");
    strcat(asm_code, "   .align 4\n");
    strcat(asm_code, "   .size ");
    strcat(asm_code, p_sym);
    strcat(asm_code, ", ");
    uint32_str_cat(asm_code, size);
    strcat(asm_code, "\n");
}

void arm_func_sym_declare_gen(char *asm_code, char *p_func) {
    strcat(asm_code, "\n");
    strcat(asm_code, ".global ");
    strcat(asm_code, p_func);
    strcat(asm_code, "\n");
    strcat(asm_code, "   .type ");
    strcat(asm_code, p_func);
    strcat(asm_code, ", %function\n");
}

void arm_push_gen(char *asm_code, size_t *reg_id, size_t num) {
    if (num == 0) return;
    strcat(asm_code, "   push {");
    strcat(asm_code, regs[reg_id[0]]);
    for (size_t i = 1; i < num; i++) {
        strcat(asm_code, ", ");
        strcat(asm_code, regs[reg_id[i]]);
    }
    strcat(asm_code, "}\n");
}

void arm_pop_gen(char *asm_code, size_t *reg_id, size_t num) {
    if (num == 0) return;
    strcat(asm_code, "   pop {");
    strcat(asm_code, regs[reg_id[0]]);
    for (size_t i = 1; i < num; i++) {
        strcat(asm_code, ", ");
        strcat(asm_code, regs[reg_id[i]]);
    }
    strcat(asm_code, "}\n");
}

void arm_get_global_addr(char *asm_code, size_t rd, char *name) {
    strcat(asm_code, "   ldr ");
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", =");
    strcat(asm_code, name);
    strcat(asm_code, "\n");
}

void arm_vmov_gen(char *asm_code, size_t rd, size_t rs) {
    bool if_sd = rd >= R_NUM;
    bool if_ss = rs >= R_NUM;
    assert(if_sd || if_ss);
    strcat(asm_code, "   vmov");
    if (if_sd && if_ss)
        strcat(asm_code, ".f32 ");
    else
        strcat(asm_code, " ");

    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", ");
    strcat(asm_code, regs[rs]);
    strcat(asm_code, "\n");
}

void arm_get_float_label_val(char *asm_code, size_t rd, char *func_name, size_t len) {
    strcat(asm_code, "   vldr.32 ");
    strcat(asm_code, regs[rd]);
    strcat(asm_code, ", .");
    strcat(asm_code, func_name);
    strcat(asm_code, "_float+");
    uint32_str_cat(asm_code, len << 2);
    strcat(asm_code, "\n");
}

void arm_float_code_gen(char *asm_code, char *func_name, char *extra_code) {
    strcat(asm_code, ".");
    strcat(asm_code, func_name);
    strcat(asm_code, "_float:\n");
    strcat(asm_code, extra_code);
}