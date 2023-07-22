
#include <backend/arm/arm_instr_gen.h>

static char regs[48][4] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc", "s0", "s1", "s2", "s3", "s4", "s5", "s6",
    "s7", "s8", "s9", "s10", "s11", "s12", "s13",
    "s14", "s15", "s16", "s17", "s18", "s19", "s20",
    "s21", "s22", "s23", "s24", "s25", "s26", "s27",
    "s28", "s29", "s30", "s31" };

static const size_t R_NUM = 16;
static const size_t S_NUM = 32;

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

static inline void uint32_str_cat(FILE *prefix, uint32_t intconst) {
    char *str = uint32_str(intconst);
    fprintf(prefix, "%s", str);
    free(str);
}

static inline void operand2str(FILE *out_file, size_t operand, size_t lsl_imme, bool if_imme) {
    if (if_imme) {
        if (lsl_imme)
            operand <<= lsl_imme;
        fprintf(out_file, "#");
        uint32_str_cat(out_file, operand);
        return;
    }
    assert(operand < R_NUM);
    fprintf(out_file, "%s", regs[operand]);
    if (lsl_imme) {
        fprintf(out_file, ", lsl #");
        uint32_str_cat(out_file, lsl_imme);
    }
}

static inline void arm_cond_type_gen(FILE *out_file, arm_cond_type cond_type) {
    switch (cond_type) {
    case arm_eq:
        fprintf(out_file, "eq ");
        break;
    case arm_ne:
        fprintf(out_file, "ne ");
        break;
    case arm_ge:
        fprintf(out_file, "ge ");
        break;
    case arm_gt:
        fprintf(out_file, "gt ");
        break;
    case arm_le:
        fprintf(out_file, "le ");
        break;
    case arm_lt:
        fprintf(out_file, "lt ");
        break;
    case arm_al:
        fprintf(out_file, " ");
        break;
    }
}

void arm_data_process_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t rs, size_t operand, bool s, size_t lsl_imme, bool if_imme) {
    assert(rd < R_NUM);
    assert(rs < R_NUM);
    switch (type) {
    case arm_add:
        fprintf(out_file, "   add");
        break;
    case arm_sub:
        fprintf(out_file, "   sub");
        break;
    case arm_rsb:
        fprintf(out_file, "   rsb");
        break;
    case arm_lsl:
        fprintf(out_file, "   lsl");
        break;
    case arm_lsr:
        fprintf(out_file, "   lsr");
        break;
    case arm_asr:
        fprintf(out_file, "   asr");
        break;
    case arm_and:
        fprintf(out_file, "   and");
        break;
    case arm_eor:
        fprintf(out_file, "   eor");
        break;
    case arm_orr:
        fprintf(out_file, "   orr");
        break;
    case arm_orn:
        fprintf(out_file, "   orn");
        break;
    default:
        assert(0);
    }
    if (s)
        fprintf(out_file, "s");
    fprintf(out_file, " ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs]);
    fprintf(out_file, ", ");
    operand2str(out_file, operand, lsl_imme, if_imme);
    fprintf(out_file, "\n");
}

// if_imme 表示 operand 是否是立即数
void arm_load_gen(FILE *out_file, size_t rd, size_t rn, size_t offset, size_t lsl_imme, bool if_imme) {
    assert(rd < R_NUM);
    assert(rn < R_NUM);
    fprintf(out_file, "   ldr ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", [");
    fprintf(out_file, "%s", regs[rn]);
    if (!(if_imme && !offset)) { // 偏移不为0
        fprintf(out_file, ", ");
        operand2str(out_file, offset, lsl_imme, if_imme);
    }
    fprintf(out_file, "]\n");
}

void arm_store_gen(FILE *out_file, size_t rd, size_t rn, size_t offset, size_t lsl_imme, bool if_imme) {
    assert(rd < R_NUM);
    assert(rn < R_NUM);
    fprintf(out_file, "   str ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", [");
    fprintf(out_file, "%s", regs[rn]);
    if (!(if_imme && !offset)) { // 偏移不为0
        fprintf(out_file, ", ");
        operand2str(out_file, offset, lsl_imme, if_imme);
    }
    fprintf(out_file, "]\n");
}

void arm_compare_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t operand, size_t lsl_imme, bool if_imme) {
    assert(rd < R_NUM);
    switch (type) {
    case arm_cmp:
        fprintf(out_file, "   cmp ");
        break;
    case arm_cmn:
        fprintf(out_file, "   cmn ");
        break;
    case arm_tst:
        fprintf(out_file, "   tst ");
        break;
    case arm_teq:
        fprintf(out_file, "   teq ");
        break;
    default:
        assert(0);
    }
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    operand2str(out_file, operand, lsl_imme, if_imme);
    fprintf(out_file, "\n");
}

void arm_mov16_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t imme16, bool s) {
    assert(rd < R_NUM);
    switch (type) {
    case arm_movw:
        fprintf(out_file, "   movw ");
        break;
    case arm_movt:
        fprintf(out_file, "   movt ");
        break;
    default:
        assert(0);
    }
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", #");
    uint32_str_cat(out_file, imme16);
    fprintf(out_file, "\n");
    if (s)
        arm_compare_gen(out_file, arm_cmp, rd, 0, 0, true);
}

void arm_mov_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t operand, bool s, size_t lsl_imme, bool if_imme) {
    assert(rd < R_NUM);
    switch (type) {
    case arm_mov:
        fprintf(out_file, "   mov");
        break;
    case arm_mvn:
        fprintf(out_file, "   mvn");
        break;
    default:
        assert(0);
    }
    if (s)
        fprintf(out_file, "s");
    fprintf(out_file, " ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    operand2str(out_file, operand, lsl_imme, if_imme);
    fprintf(out_file, "\n");
}

void arm_movcond_gen(FILE *out_file, arm_cond_type cond_type, size_t rd, size_t operand, size_t lsl_imme, bool if_imme) {
    assert(rd < R_NUM);
    fprintf(out_file, "   mov");
    arm_cond_type_gen(out_file, cond_type);
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    operand2str(out_file, operand, lsl_imme, if_imme);
    fprintf(out_file, "\n");
}

void arm_mul_gen(FILE *out_file, size_t rd, size_t rm, size_t rs, bool s) {
    assert(rd < R_NUM);
    assert(rm < R_NUM);
    assert(rs < R_NUM);
    fprintf(out_file, "   mul");
    if (s)
        fprintf(out_file, "s");
    fprintf(out_file, " ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rm]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs]);
    fprintf(out_file, "\n");
}

void arm_sdiv_gen(FILE *out_file, size_t rd, size_t rm, size_t rs) {
    assert(rd < R_NUM);
    assert(rm < R_NUM);
    assert(rs < R_NUM);
    fprintf(out_file, "  sdiv ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rm]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs]);
    fprintf(out_file, "\n");
}

void arm_label_gen(FILE *out_file, char *name) {
    fprintf(out_file, "%s", name);
    fprintf(out_file, ":\n");
}

void arm_jump_reg_gen(FILE *out_file, arm_instr_type type, arm_cond_type cond_type, size_t r_target) {
    assert(r_target < R_NUM);
    switch (type) {
    case arm_bx:
        fprintf(out_file, "   bx");
        break;
    case arm_blx:
        fprintf(out_file, "   blx");
        break;
    default:
        assert(0);
        break;
    }
    arm_cond_type_gen(out_file, cond_type);
    fprintf(out_file, "%s", regs[r_target]);
    fprintf(out_file, "\n");
}

void arm_jump_label_gen(FILE *out_file, arm_instr_type type, arm_cond_type cond_type, char *label) {
    switch (type) {
    case arm_b:
        fprintf(out_file, "   b");
        break;
    case arm_bl:
        fprintf(out_file, "   bl");
        break;
    default:
        assert(0);
    }
    arm_cond_type_gen(out_file, cond_type);
    fprintf(out_file, "%s", label);
    fprintf(out_file, "\n");
}

void arm_swap_gen(FILE *out_file, size_t r1, size_t r2) {
    if (r1 == r2) return;
    bool if_r1 = r1 < R_NUM;
    bool if_r2 = r2 < R_NUM;
    assert(if_r1 == if_r2);
    if (if_r1 && if_r2) {
        arm_data_process_gen(out_file, arm_eor, r1, r1, r2, false, 0, false);
        arm_data_process_gen(out_file, arm_eor, r2, r1, r2, false, 0, false);
        arm_data_process_gen(out_file, arm_eor, r1, r1, r2, false, 0, false);
    }
    else {
        arm_vdata_process_gen(out_file, arm_add, r1, r1, r2);
        arm_vdata_process_gen(out_file, arm_sub, r2, r1, r2);
        arm_vdata_process_gen(out_file, arm_sub, r1, r1, r2);
    }
}

void arm_word_gen(FILE *out_file, size_t imme32) {
    fprintf(out_file, "   .word ");
    uint32_str_cat(out_file, imme32);
    fprintf(out_file, "\n");
}

void arm_space_gen(FILE *out_file, size_t size) {
    fprintf(out_file, "   .space ");
    uint32_str_cat(out_file, size);
    fprintf(out_file, "\n");
}

void arm_global_sym_declare_gen(FILE *out_file, char *p_sym, size_t size) {
    fprintf(out_file, "\n");
    fprintf(out_file, ".global ");
    fprintf(out_file, "%s", p_sym);
    fprintf(out_file, "\n");
    fprintf(out_file, "   .type ");
    fprintf(out_file, "%s", p_sym);
    fprintf(out_file, ", %%object\n");
    fprintf(out_file, "   .data\n");
    fprintf(out_file, "   .align 4\n");
    fprintf(out_file, "   .size ");
    fprintf(out_file, "%s", p_sym);
    fprintf(out_file, ", ");
    uint32_str_cat(out_file, size);
    fprintf(out_file, "\n");
}

void arm_func_sym_declare_gen(FILE *out_file, char *p_func) {
    fprintf(out_file, "\n");
    fprintf(out_file, ".global ");
    fprintf(out_file, "%s", p_func);
    fprintf(out_file, "\n");
    fprintf(out_file, "   .type ");
    fprintf(out_file, "%s", p_func);
    fprintf(out_file, ", %%function\n");
}

static inline int cmp(const void *a, const void *b) {
    return *(int *) a - *(int *) b;
}

void arm_push_gen(FILE *out_file, size_t *reg_id, size_t num) {
    if (num == 0) return;
    assert(reg_id[0] < R_NUM);
    qsort(reg_id, num, sizeof(size_t), cmp);
    fprintf(out_file, "   push {");
    fprintf(out_file, "%s", regs[reg_id[0]]);
    for (size_t i = 1; i < num; i++) {
        assert(reg_id[i] < R_NUM);
        fprintf(out_file, ", ");
        fprintf(out_file, "%s", regs[reg_id[i]]);
    }
    fprintf(out_file, "}\n");
}

void arm_pop_gen(FILE *out_file, size_t *reg_id, size_t num) {
    if (num == 0) return;
    assert(reg_id[0] < R_NUM);
    qsort(reg_id, num, sizeof(size_t), cmp);
    fprintf(out_file, "   pop {");
    fprintf(out_file, "%s", regs[reg_id[0]]);
    for (size_t i = 1; i < num; i++) {
        assert(reg_id[i] < R_NUM);
        fprintf(out_file, ", ");
        fprintf(out_file, "%s", regs[reg_id[i]]);
    }
    fprintf(out_file, "}\n");
}

void arm_get_global_addr(FILE *out_file, size_t rd, char *name, size_t offset) {
    assert(rd < R_NUM);
    fprintf(out_file, "   movw ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", #:lower16:");
    fprintf(out_file, "%s", name);
    if (offset)
        fprintf(out_file, "+%ld", offset);
    fprintf(out_file, "\n");
    fprintf(out_file, "   movt ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", #:upper16:");
    fprintf(out_file, "%s", name);
    if (offset)
        fprintf(out_file, "+%ld", offset);
    fprintf(out_file, "\n");
}

void arm_vset_flag(FILE *out_file, size_t r) {
    assert(r >= R_NUM);
    assert(r < R_NUM + S_NUM);
    fprintf(out_file, "   vcmp.f32 ");
    fprintf(out_file, "%s", regs[r]);
    fprintf(out_file, ", #0\n");
    fprintf(out_file, "   vmrs APSR_nzcv, FPSCR\n");
}

void arm_vmov_gen(FILE *out_file, size_t rd, size_t rs) {
    bool if_sd = rd >= R_NUM;
    bool if_ss = rs >= R_NUM;
    assert(if_sd || if_ss);
    fprintf(out_file, "   vmov");
    if (if_sd && if_ss)
        fprintf(out_file, ".f32 ");
    else
        fprintf(out_file, " ");

    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs]);
    fprintf(out_file, "\n");
}

void arm_vneg_gen(FILE *out_file, size_t rd, size_t rs) {
    assert(rs >= R_NUM);
    assert(rs < R_NUM + S_NUM);
    assert(rd >= R_NUM);
    assert(rd < R_NUM + S_NUM);
    fprintf(out_file, "   vneg.f32 ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs]);
    fprintf(out_file, "\n");
}

void arm_vcvt_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t rs) {
    assert(rd >= R_NUM);
    assert(rd < R_NUM + S_NUM);
    assert(rs >= R_NUM);
    assert(rs < R_NUM + S_NUM);
    fprintf(out_file, "   vcvt");
    switch (type) {
    case arm_int2float:
        fprintf(out_file, ".f32.s32 ");
        break;
    case arm_float2int:
        fprintf(out_file, ".s32.f32 ");
        break;
    default:
        assert(0);
    }
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs]);
    fprintf(out_file, "\n");
}

void arm_vdata_process_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t rs1, size_t rs2) {
    assert(rd >= R_NUM);
    assert(rd < R_NUM + S_NUM);
    assert(rs1 >= R_NUM);
    assert(rs1 < R_NUM + S_NUM);
    assert(rs2 >= R_NUM);
    assert(rs2 < R_NUM + S_NUM);
    switch (type) {
    case arm_add:
        fprintf(out_file, "   vadd");
        break;
    case arm_sub:
        fprintf(out_file, "   vsub");
        break;
    case arm_mul:
        fprintf(out_file, "   vmul");
        break;
    case arm_div:
        fprintf(out_file, "   vdiv");
        break;
    case arm_and:
        fprintf(out_file, "   vand");
        break;
    default:
        assert(0);
    }
    fprintf(out_file, ".f32 ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs1]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs2]);
    fprintf(out_file, "\n");
}

void arm_vcompare_gen(FILE *out_file, arm_instr_type type, size_t rs1, size_t rs2) {
    assert(rs1 >= R_NUM);
    assert(rs1 < R_NUM + S_NUM);
    assert(rs2 >= R_NUM);
    assert(rs2 < R_NUM + S_NUM);
    switch (type) {
    case arm_cmp:
        fprintf(out_file, "  vcmp");
        break;
    case arm_tst:
        fprintf(out_file, "  vtst");
        break;
    default:
        assert(0);
        break;
    }
    fprintf(out_file, ".f32 ");
    fprintf(out_file, "%s", regs[rs1]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs2]);
    fprintf(out_file, "\n");
    fprintf(out_file, "   vmrs APSR_nzcv, FPSCR\n");
}

void arm_vload_gen(FILE *out_file, size_t rd, size_t rs, size_t offset) {
    assert(rd >= R_NUM);
    assert(rd < R_NUM + S_NUM);
    assert(rs < R_NUM);
    fprintf(out_file, "   vldr.32 ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", [");
    fprintf(out_file, "%s", regs[rs]);
    if (offset) {
        fprintf(out_file, ", #");
        uint32_str_cat(out_file, offset);
    }
    fprintf(out_file, "]\n");
}

void arm_vstore_gen(FILE *out_file, size_t rd, size_t rs, size_t offset) {
    assert(rd >= R_NUM);
    assert(rd < R_NUM + S_NUM);
    assert(rs < R_NUM);
    fprintf(out_file, "   vstr.32 ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", [");
    fprintf(out_file, "%s", regs[rs]);
    if (offset) {
        fprintf(out_file, ", #");
        uint32_str_cat(out_file, offset);
    }
    fprintf(out_file, "]\n");
}

void arm_vpush_gen(FILE *out_file, size_t *reg_id, size_t num) {
    if (num == 0) return;
    qsort(reg_id, num, sizeof(size_t), cmp);
    size_t i = 0;
    while (i < num) {
        assert(reg_id[i] >= R_NUM);
        assert(reg_id[i] < R_NUM + S_NUM);
        fprintf(out_file, "   vpush {");
        fprintf(out_file, "%s", regs[reg_id[i]]);
        i++;
        while (i < num && reg_id[i] == reg_id[i - 1] + 1) {
            assert(reg_id[i] >= R_NUM);
            assert(reg_id[i] < R_NUM + S_NUM);
            fprintf(out_file, ", ");
            fprintf(out_file, "%s", regs[reg_id[i]]);
            i++;
        }
        fprintf(out_file, "}\n");
    }
}
void arm_vpop_gen(FILE *out_file, size_t *reg_id, size_t num) {
    if (num == 0) return;
    qsort(reg_id, num, sizeof(size_t), cmp);
    size_t i = 0;
    while (i < num) {
        assert(reg_id[i] >= R_NUM);
        assert(reg_id[i] < R_NUM + S_NUM);
        fprintf(out_file, "   vpop {");
        fprintf(out_file, "%s", regs[reg_id[i]]);
        i++;
        while (i < num && reg_id[i] == reg_id[i - 1] + 1) {
            assert(reg_id[i] >= R_NUM);
            assert(reg_id[i] < R_NUM + S_NUM);
            fprintf(out_file, ", ");
            fprintf(out_file, "%s", regs[reg_id[i]]);
            i++;
        }
        fprintf(out_file, "}\n");
    }
}
