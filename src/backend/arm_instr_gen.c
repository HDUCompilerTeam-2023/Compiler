
#include <backend/arm/arm_instr_gen.h>
#include <ir_opt/lir_gen/arm_standard.h>

static char regs[49][4] = { "a0", "a1", "a2", "a3", "a4", "a5", "a6", "s2",
    "s3", "s4", "s5", "s6", "s7", "sp", "ra", "pc", "f10", "f11", "f12", "f13", "f14", "f15", "f16",
    "f17", "f18", "f19", "f20", "f21", "f22", "f23",
    "f24", "f25", "f26", "f27", "f28", "f29", "f30",
    "f31", "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9" };

// static const size_t R_NUM = 16;
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

static inline void uint32_str_cat(FILE *prefix, uint32_t intconst) {
    char *str = uint32_str(intconst);
    fprintf(prefix, "%s", str);
    free(str);
}

// static inline void operand2str(FILE *out_file, size_t operand, size_t lsl_imme, bool if_imme) {
//     if (if_imme) {
//         if (lsl_imme)
//             operand <<= lsl_imme;
//         fprintf(out_file, "#");
//         uint32_str_cat(out_file, operand);
//         return;
//     }
//     assert(operand < R_NUM);
//     fprintf(out_file, "%s", regs[operand]);
//     if (lsl_imme) {
//         fprintf(out_file, ", lsl #");
//         uint32_str_cat(out_file, lsl_imme);
//     }
// }

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
void arm_li_gen(FILE *out_file, size_t rd, size_t imme){
    fprintf(out_file, "   li %s, ", regs[rd]);
    fprintf(out_file, " %ld\n", imme);
}
void arm_mov_gen(FILE *out_file, size_t rd, size_t rs){
    fprintf(out_file, "   mv %s, %s\n", regs[rd], regs[rd]);
}
void arm_mvn_gen(FILE *out_file, size_t rd, size_t rs){
    fprintf(out_file, "   not %s, %s\n", regs[rd], regs[rd]);
}
void arm_set_gen(FILE *out_file, size_t rd, size_t rs, arm_cond_type cond_type ){
    fprintf(out_file, "s");
    arm_cond_type_gen(out_file, cond_type);
    fprintf(out_file, "z");
    fprintf(out_file, " %s", regs[rd]);
    fprintf(out_file, " %s\n", regs[rs]);
}
void arm_addi_gen(FILE *out_file, size_t rd, size_t rs1, int offset){
    fprintf(out_file, "   addi %s, %s, %d\n", regs[rd], regs[rs1], offset);
}
void arm_data_process_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t rs1, size_t rs2) {
    assert(rd < R_NUM);
    assert(rs1 < R_NUM);
    assert(rs2 < R_NUM);
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
    fprintf(out_file, " ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs1]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs2]);
    fprintf(out_file, "\n");
}

// if_imme 表示 operand 是否是立即数
void arm_load_gen(FILE *out_file, size_t rd, size_t rn, size_t offset) {
    assert(rd < R_NUM);
    assert(rn < R_NUM);
    arm_li_gen(out_file, TMP, offset);
    arm_data_process_gen(out_file, arm_add, rn, TMP, rn);
    fprintf(out_file, "   ld ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", %d", 0);
    fprintf(out_file, "(%s)\n", regs[rn]);
}

void arm_store_gen(FILE *out_file, size_t rd, size_t rn, size_t offset) {
    assert(rd < R_NUM);
    assert(rn < R_NUM);
    arm_li_gen(out_file, TMP, offset);
    arm_data_process_gen(out_file, arm_add, rn, TMP, rn);
    fprintf(out_file, "   sd ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", %d", 0);
    fprintf(out_file, "(%s)\n", regs[rn]);
}

void arm_mul_gen(FILE *out_file, size_t rd, size_t rm, size_t rs) {
    assert(rd < R_NUM);
    assert(rm < R_NUM);
    assert(rs < R_NUM);
    fprintf(out_file, "   mul");
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
    fprintf(out_file, "  div ");
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

void arm_call_gen(FILE *out_file, char *name){
    fprintf(out_file, "   call %s\n", name);
}
void arm_jump_label_gen(FILE *out_file, arm_instr_type type, arm_cond_type cond_type, size_t rs1, size_t rs2, char *label) {
    if(rs1 >=R_NUM){
        fprintf(out_file, "     f");
        switch (cond_type) {
        case arm_eq:
            fprintf(out_file, "eq");
            break;
        case arm_ne:
            fprintf(out_file, "ne");
            break;
        case arm_ge:
            fprintf(out_file, "ge");
            break;
        case arm_gt:
            fprintf(out_file, "gt");
            break;
        case arm_le:
            fprintf(out_file, "le");
            break;
        case arm_lt:
            fprintf(out_file, "lt");
            break;
        case arm_al:
            fprintf(out_file, "");
            break;
        }
        fprintf(out_file, ".s %s, %s, %s\n", regs[TMP], regs[rs1], regs[rs2]);
        fprintf(out_file, "bnez %s, %s\n", regs[TMP], label);
        return;
    }
    switch (type) {
    case arm_b:
        fprintf(out_file, "   b");
        break;
    case arm_bl:
        fprintf(out_file, "   j");
        break;
    default:
        assert(0);
    }
    if(type != arm_bl){
        arm_cond_type_gen(out_file, cond_type);
        fprintf(out_file, " %s, ", regs[rs1]);
        fprintf(out_file, "%s, ", regs[rs2]);
    }
    fprintf(out_file, " %s", label);
    fprintf(out_file, "\n");
}

// void arm_swap_gen(FILE *out_file, size_t r1, size_t r2) {
//     if (r1 == r2) return;
//     bool if_r1 = r1 < R_NUM;
//     bool if_r2 = r2 < R_NUM;
//     assert(if_r1 == if_r2);
//     if (if_r1 && if_r2) {
//         arm_data_process_gen(out_file, arm_eor, r1, r1, r2);
//         arm_data_process_gen(out_file, arm_eor, r2, r1, r2, false, 0, false);
//         arm_data_process_gen(out_file, arm_eor, r1, r1, r2, false, 0, false);
//     }
//     else {
//         arm_vdata_process_gen(out_file, arm_add, r1, r1, r2);
//         arm_vdata_process_gen(out_file, arm_sub, r2, r1, r2);
//         arm_vdata_process_gen(out_file, arm_sub, r1, r1, r2);
//     }
// }

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
    fprintf(out_file, ".globl ");
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
    fprintf(out_file, ".globl ");
    fprintf(out_file, "%s", p_func);
    fprintf(out_file, "\n");
    fprintf(out_file, "   .type ");
    fprintf(out_file, "%s", p_func);
    fprintf(out_file, ", @function\n");
}


void arm_get_global_addr(FILE *out_file, size_t rd, char *name, size_t offset) {
    bool small_off = offset && offset < 32768;
    assert(rd < R_NUM);
    fprintf(out_file, "   lla ");
    fprintf(out_file, "%s, ", regs[rd]);
    fprintf(out_file, "%s\n", name);
    if(small_off){
        arm_li_gen(out_file, TMP, offset);
        arm_data_process_gen(out_file, arm_add, rd, rd, TMP);
    }
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
    fprintf(out_file, "   fmv");
    if (if_sd && if_ss)
        fprintf(out_file, ".s ");
    if (if_sd && !if_ss)
        fprintf(out_file, ".w.x ");
    if (!if_sd && if_ss)
        fprintf(out_file, ".x.w ");

    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs]);
    fprintf(out_file, "\n");
}
void arm_neg_gen(FILE *out_file, size_t rd, size_t rs) {
    fprintf(out_file, "   neg ");
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
    fprintf(out_file, "   fneg.s ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs]);
    fprintf(out_file, "\n");
}

void arm_vcvt_gen(FILE *out_file, arm_instr_type type, size_t rd, size_t rs) {
    // assert(rd >= R_NUM);
    // assert(rd < R_NUM + S_NUM);
    // assert(rs  R_NUM);
    // assert(rs < R_NUM + S_NUM);
    fprintf(out_file, "   fcvt");
    switch (type) {
    case arm_int2float:
        fprintf(out_file, ".s.w ");
        break;
    case arm_float2int:
        fprintf(out_file, ".w.s ");
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
        fprintf(out_file, "   fadd.s");
        break;
    case arm_sub:
        fprintf(out_file, "   fsub.s");
        break;
    case arm_mul:
        fprintf(out_file, "   fmul.s");
        break;
    case arm_div:
        fprintf(out_file, "   fdiv.s");
        break;
    case arm_and:
        fprintf(out_file, "   fand.s");
        break;
    default:
        assert(0);
    }
    fprintf(out_file, " %s", regs[rd]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs1]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs2]);
    fprintf(out_file, "\n");
}

void arm_vcompare_gen(FILE *out_file, arm_instr_type type, arm_cond_type cond_type, size_t rs1, size_t rs2) {
    assert(rs1 >= R_NUM);
    assert(rs1 < R_NUM + S_NUM);
    assert(rs2 >= R_NUM);
    assert(rs2 < R_NUM + S_NUM);
    switch (type) {
    case arm_cmp:
        fprintf(out_file, "  f");
        break;
    case arm_tst:
        fprintf(out_file, "  vtst");
        break;
    default:
        assert(0);
        break;
    }
    arm_cond_type_gen(out_file, cond_type);
    fprintf(out_file, ".s %s", regs[rs1]);
    fprintf(out_file, ", ");
    fprintf(out_file, "%s", regs[rs2]);
    fprintf(out_file, "\n");
}

void arm_vload_gen(FILE *out_file, size_t rd, size_t rs, size_t offset) {
    assert(rd >= R_NUM);
    assert(rd < R_NUM + S_NUM);
    assert(rs < R_NUM);
    arm_li_gen(out_file, TMP, offset);
    arm_data_process_gen(out_file, arm_add, rs, TMP, rs);
    fprintf(out_file, "   fld ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", %d", 0);
    fprintf(out_file, "(%s)\n", regs[rs]);
}

void arm_vstore_gen(FILE *out_file, size_t rd, size_t rs, size_t offset) {
    assert(rd >= R_NUM);
    assert(rd < R_NUM + S_NUM);
    assert(rs < R_NUM);
    arm_li_gen(out_file, TMP, offset);
    arm_data_process_gen(out_file, arm_add, rs, TMP, rs);
    fprintf(out_file, "   fsd ");
    fprintf(out_file, "%s", regs[rd]);
    fprintf(out_file, ", %d", 0);
    fprintf(out_file, "(%s)\n", regs[rs]);
}

