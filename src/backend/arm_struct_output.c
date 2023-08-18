#include <backend/arm/arm_struct_output.h>
#include <symbol/type.h>
#include <symbol/var.h>
static char regs[48][4] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc", "s0", "s1", "s2", "s3", "s4", "s5", "s6",
    "s7", "s8", "s9", "s10", "s11", "s12", "s13",
    "s14", "s15", "s16", "s17", "s18", "s19", "s20",
    "s21", "s22", "s23", "s24", "s25", "s26", "s27",
    "s28", "s29", "s30", "s31" };

static inline void arm_reg_output(FILE *out_file, arm_reg reg) {
    fprintf(out_file, "%s", regs[reg]);
}
static inline void arm_word_output(FILE *out_file, arm_imme imme32) {
    fprintf(out_file, "   .word ");
    fprintf(out_file, "%d", imme32);
    fprintf(out_file, "\n");
}

static inline void arm_space_output(FILE *out_file, arm_imme size) {
    fprintf(out_file, "   .space ");
    fprintf(out_file, "%d", size);
    fprintf(out_file, "\n");
}

static inline void arm_func_sym_declare_output(FILE *out_file, arm_label p_func) {
    fprintf(out_file, "\n");
    fprintf(out_file, ".global ");
    fprintf(out_file, "%s", p_func);
    fprintf(out_file, "\n");
    fprintf(out_file, "   .type ");
    fprintf(out_file, "%s", p_func);
    fprintf(out_file, ", %%function\n");
}
static inline void arm_global_sym_declare_output(FILE *out_file, arm_label p_sym, arm_imme size) {
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
    fprintf(out_file, "%d", size);
    fprintf(out_file, "\n");
}
static inline void operand2str(FILE *out_file, p_arm_operand p_operand) {
    if (p_operand->if_imme) {
        fprintf(out_file, "#");
        fprintf(out_file, "%d", p_operand->imme);
        return;
    }
    arm_reg_output(out_file, p_operand->reg);
    if (p_operand->lsl_imme) {
        fprintf(out_file, ", lsl #");
        fprintf(out_file, "%d", p_operand->lsl_imme);
    }
}

static inline void arm_cond_type_output(FILE *out_file, arm_cond_type cond_type) {
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
static inline void arm_label_output(FILE *out_file, arm_label name) {
    fprintf(out_file, "%s", name);
    fprintf(out_file, ":\n");
}

static inline void arm_binary_instr_output(FILE *out_file, p_arm_binary_instr p_binary_instr) {
    switch (p_binary_instr->op) {
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
    if (p_binary_instr->s)
        fprintf(out_file, "s");
    fprintf(out_file, " ");
    arm_reg_output(out_file, p_binary_instr->rd);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_binary_instr->rs1);
    fprintf(out_file, ", ");
    operand2str(out_file, p_binary_instr->op2);
    fprintf(out_file, "\n");
}

static inline void arm_jump_instr_output(FILE *out_file, p_arm_jump_instr p_jump_instr) {
    bool if_label = true;
    switch (p_jump_instr->op) {
    case arm_b:
        fprintf(out_file, "   b");
        break;
    case arm_bl:
        fprintf(out_file, "   bl");
        break;
    case arm_bx:
        if_label = false;
        fprintf(out_file, "   bx");
        break;
    case arm_blx:
        if_label = false;
        fprintf(out_file, "   blx");
        break;
    default:
        assert(0);
        break;
    }
    arm_cond_type_output(out_file, p_jump_instr->cond_type);
    if (if_label)
        fprintf(out_file, "%s", p_jump_instr->p_block_target->label);
    else
        arm_reg_output(out_file, p_jump_instr->reg_target);
    fprintf(out_file, "\n");
}
static inline void arm_call_instr_output(FILE *out_file, p_arm_call_instr p_call_instr) {
    fprintf(out_file, "   bl %s\n", p_call_instr->func_name);
}
static inline void arm_cmp_instr_output(FILE *out_file, p_arm_cmp_instr p_cmp_instr) {
    switch (p_cmp_instr->op) {
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
    arm_reg_output(out_file, p_cmp_instr->rs1);
    fprintf(out_file, ", ");
    operand2str(out_file, p_cmp_instr->op2);
    fprintf(out_file, "\n");
}

static inline void arm_mem_instr_output(FILE *out_file, p_arm_mem_instr p_mem_instr) {
    switch (p_mem_instr->op) {
    case arm_store:
        fprintf(out_file, "   str ");
        break;
    case arm_load:
        fprintf(out_file, "   ldr ");
        break;
    }
    arm_reg_output(out_file, p_mem_instr->rd);
    fprintf(out_file, ", [");
    arm_reg_output(out_file, p_mem_instr->base);
    if (!(p_mem_instr->offset->if_imme && !p_mem_instr->offset->imme)) { // 偏移不为0
        fprintf(out_file, ", ");
        operand2str(out_file, p_mem_instr->offset);
    }
    fprintf(out_file, "]\n");
}
static inline void arm_mov_instr_output(FILE *out_file, p_arm_mov_instr p_mov_instr) {
    switch (p_mov_instr->op) {
    case arm_mov:
        fprintf(out_file, "   mov");
        break;
    case arm_mvn:
        fprintf(out_file, "   mvn");
        break;
    default:
        assert(0);
    }
    if (p_mov_instr->s)
        fprintf(out_file, "s");
    arm_cond_type_output(out_file, p_mov_instr->type);
    arm_reg_output(out_file, p_mov_instr->rd);
    fprintf(out_file, ", ");
    operand2str(out_file, p_mov_instr->operand);
    fprintf(out_file, "\n");
}
static inline void arm_mov32_instr_output(FILE *out_file, p_arm_mov32_instr p_mov32_instr) {
    if (p_mov32_instr->label) {
        bool small_off = p_mov32_instr->imme && p_mov32_instr->imme < 32768;
        fprintf(out_file, "   movw ");
        arm_reg_output(out_file, p_mov32_instr->rd);
        fprintf(out_file, ", #:lower16:");
        fprintf(out_file, "%s", p_mov32_instr->label);
        if (small_off)
            fprintf(out_file, "+%d", p_mov32_instr->imme);
        fprintf(out_file, "\n");
        fprintf(out_file, "   movt ");
        arm_reg_output(out_file, p_mov32_instr->rd);
        fprintf(out_file, ", #:upper16:");
        fprintf(out_file, "%s", p_mov32_instr->label);
        if (small_off)
            fprintf(out_file, "+%d", p_mov32_instr->imme);
        fprintf(out_file, "\n");
    }
    else {
        arm_imme low = ((1 << 16) - 1) & p_mov32_instr->imme;
        arm_imme high = (p_mov32_instr->imme >> 16);
        fprintf(out_file, "   movw ");
        arm_reg_output(out_file, p_mov32_instr->rd);
        fprintf(out_file, ", #");
        fprintf(out_file, "%d", low);
        fprintf(out_file, "\n");
        fprintf(out_file, "   movt ");
        arm_reg_output(out_file, p_mov32_instr->rd);
        fprintf(out_file, ", #");
        fprintf(out_file, "%d", high);
        fprintf(out_file, "\n");
    }
}

static inline void arm_mul_instr_output(FILE *out_file, p_arm_mul_instr p_mul_instr) {
    fprintf(out_file, "   mul");
    if (p_mul_instr->s)
        fprintf(out_file, "s");
    fprintf(out_file, " ");
    arm_reg_output(out_file, p_mul_instr->rd);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_mul_instr->rs1);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_mul_instr->rs2);
    fprintf(out_file, "\n");
}

static inline void arm_sdiv_instr_output(FILE *out_file, p_arm_sdiv_instr p_sdiv_instr) {
    fprintf(out_file, "   sdiv");
    fprintf(out_file, " ");
    arm_reg_output(out_file, p_sdiv_instr->rd);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_sdiv_instr->rs1);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_sdiv_instr->rs2);
    fprintf(out_file, "\n");
}

static inline void arm_push_pop_instr_output(FILE *out_file, p_arm_push_pop_instr p_push_pop) {
    if (p_push_pop->num == 0) return;
    switch (p_push_pop->op) {
    case arm_push:
        fprintf(out_file, "   push {");
        break;
    case arm_pop:
        fprintf(out_file, "   pop {");
        break;
    default:
        assert(0);
    }
    arm_reg_output(out_file, p_push_pop->regs[0]);
    for (size_t i = 1; i < p_push_pop->num; i++) {
        fprintf(out_file, ", ");
        arm_reg_output(out_file, p_push_pop->regs[i]);
    }
    fprintf(out_file, "}\n");
}
static inline void arm_vbinary_instr_output(FILE *out_file, p_arm_vbinary_instr p_vbinary_instr) {
    switch (p_vbinary_instr->op) {
    case arm_vadd:
        fprintf(out_file, "   vadd");
        break;
    case arm_vsub:
        fprintf(out_file, "   vsub");
        break;
    case arm_vmul:
        fprintf(out_file, "   vmul");
        break;
    case arm_vdiv:
        fprintf(out_file, "   vdiv");
        break;
    case arm_vand:
        fprintf(out_file, "   vand");
        break;
    }
    fprintf(out_file, ".f32 ");
    arm_reg_output(out_file, p_vbinary_instr->rd);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_vbinary_instr->rs1);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_vbinary_instr->rs2);
    fprintf(out_file, "\n");
}
static inline void arm_vmem_instr_output(FILE *out_file, p_arm_vmem_instr p_vmem_instr) {
    switch (p_vmem_instr->op) {
    case arm_vstore:
        fprintf(out_file, "   vstr.32 ");
        break;
    case arm_vload:
        fprintf(out_file, "   vldr.32 ");
        break;
    default:
        assert(0);
    }
    arm_reg_output(out_file, p_vmem_instr->rd);
    fprintf(out_file, ", [");
    arm_reg_output(out_file, p_vmem_instr->base);
    if (p_vmem_instr->offset) { // 偏移不为0
        fprintf(out_file, ", #");
        fprintf(out_file, "%d", p_vmem_instr->offset);
    }
    fprintf(out_file, "]\n");
}
static inline void arm_vmov_instr_output(FILE *out_file, p_arm_vmov_instr p_vmov_instr) {
    fprintf(out_file, "   vmov");
    if (p_vmov_instr->rd >= R_NUM && p_vmov_instr->rs >= R_NUM)
        fprintf(out_file, ".f32 ");
    else
        fprintf(out_file, " ");
    arm_reg_output(out_file, p_vmov_instr->rd);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_vmov_instr->rs);
    fprintf(out_file, "\n");
}
static inline void arm_vcmp_instr_output(FILE *out_file, p_arm_vcmp_instr p_vcmp_instr) {
    switch (p_vcmp_instr->op) {
    case arm_vcmp:
        fprintf(out_file, "  vcmp");
        break;
    case arm_vtst:
        fprintf(out_file, "  vtst");
        break;
    default:
        assert(0);
        break;
    }
    fprintf(out_file, ".f32 ");
    arm_reg_output(out_file, p_vcmp_instr->rs1);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_vcmp_instr->rs2);
    fprintf(out_file, "\n");
    fprintf(out_file, "   vmrs APSR_nzcv, FPSCR\n");
}
static inline void arm_vcvt_instr_output(FILE *out_file, p_arm_vcvt_instr p_vcvt_instr) {
    fprintf(out_file, "   vcvt");
    switch (p_vcvt_instr->op) {
    case arm_int2float:
        fprintf(out_file, ".f32.s32 ");
        break;
    case arm_float2int:
        fprintf(out_file, ".s32.f32 ");
        break;
    default:
        assert(0);
    }
    arm_reg_output(out_file, p_vcvt_instr->rd);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_vcvt_instr->rs);
    fprintf(out_file, "\n");
}
static inline void arm_vneg_instr_output(FILE *out_file, p_arm_vneg_instr p_vneg_instr) {
    fprintf(out_file, "   vneg.f32 ");
    arm_reg_output(out_file, p_vneg_instr->rd);
    fprintf(out_file, ", ");
    arm_reg_output(out_file, p_vneg_instr->rs);
    fprintf(out_file, "\n");
}

static inline void arm_vpush_vpop_instr_output(FILE *out_file, p_arm_vpush_vpop_instr p_vpush_vpop) {
    if (p_vpush_vpop->num == 0) return;
    switch (p_vpush_vpop->op) {
    case arm_vpush:
        fprintf(out_file, "   vpush {");
        break;
    case arm_vpop:
        fprintf(out_file, "   vpop {");
        break;
    default:
        assert(0);
    }
    arm_reg_output(out_file, p_vpush_vpop->regs[0]);
    for (size_t i = 1; i < p_vpush_vpop->num; i++) {
        fprintf(out_file, ", ");
        arm_reg_output(out_file, p_vpush_vpop->regs[i]);
    }
    fprintf(out_file, "}\n");
}
static inline void arm_instr_output(FILE *out_file, p_arm_instr p_instr) {
    switch (p_instr->type) {
    case arm_binary_type:
        arm_binary_instr_output(out_file, &p_instr->binary_instr);
        break;
    case arm_jump_type:
        arm_jump_instr_output(out_file, &p_instr->jump_instr);
        break;
    case arm_call_type:
        arm_call_instr_output(out_file, &p_instr->call_instr);
        break;
    case arm_cmp_type:
        arm_cmp_instr_output(out_file, &p_instr->cmp_instr);
        break;
    case arm_mem_type:
        arm_mem_instr_output(out_file, &p_instr->mem_instr);
        break;
    case arm_mov_type:
        arm_mov_instr_output(out_file, &p_instr->mov_instr);
        break;
    case arm_mov32_type:
        arm_mov32_instr_output(out_file, &p_instr->mov32_instr);
        break;
    case arm_mul_type:
        arm_mul_instr_output(out_file, &p_instr->mul_instr);
        break;
    case arm_sdiv_type:
        arm_sdiv_instr_output(out_file, &p_instr->sdiv_instr);
        break;
    case arm_push_pop_type:
        arm_push_pop_instr_output(out_file, &p_instr->push_pop_instr);
        break;
    case arm_vbinary_type:
        arm_vbinary_instr_output(out_file, &p_instr->vbinary_instr);
        break;
    case arm_vmov_type:
        arm_vmov_instr_output(out_file, &p_instr->vmov_instr);
        break;
    case arm_vmem_type:
        arm_vmem_instr_output(out_file, &p_instr->vmem_instr);
        break;
    case arm_vcvt_type:
        arm_vcvt_instr_output(out_file, &p_instr->vcvt_instr);
        break;
    case arm_vneg_type:
        arm_vneg_instr_output(out_file, &p_instr->vneg_instr);
        break;
    case arm_vcmp_type:
        arm_vcmp_instr_output(out_file, &p_instr->vcmp_instr);
        break;
    case arm_vpush_vpop_type:
        arm_vpush_vpop_instr_output(out_file, &p_instr->vpush_vpop_instr);
        break;
    }
}

static inline void arm_block_output(FILE *out_file, p_arm_block p_block) {
    arm_label_output(out_file, p_block->label);
    p_list_head p_node;
    list_for_each(p_node, &p_block->instr_list) {
        p_arm_instr p_instr = list_entry(p_node, arm_instr, node);
        arm_instr_output(out_file, p_instr);
    }
}

void arm_func_output(FILE *out_file, p_arm_func p_func) {
    arm_func_sym_declare_output(out_file, p_func->func_name);
    arm_block_output(out_file, p_func->p_into_func_block);
    p_list_head p_node;
    list_for_each(p_node, &p_func->block_list) {
        p_arm_block p_block = list_entry(p_node, arm_block, node);
        arm_block_output(out_file, p_block);
    }
}
void arm_global_sym_output(FILE *out_file, p_symbol_var p_sym) {
    if (!p_sym->p_init) {
        fprintf(out_file, "\n.comm %s,%ld,4\n", p_sym->name, p_sym->p_type->size * basic_type_get_size(p_sym->p_type->basic));
        return;
    }
    arm_global_sym_declare_output(out_file, p_sym->name, p_sym->p_type->size * basic_type_get_size(p_sym->p_type->basic));
    arm_label_output(out_file, p_sym->name);
    size_t i = 0;
    while (i < p_sym->p_init->size) {
        size_t space_loc = i;
        while (i < p_sym->p_init->size && p_sym->p_init->memory[i].f == 0)
            i++;
        size_t space_size = i - space_loc;
        if (space_size)
            arm_space_output(out_file, space_size << 2);
        if (i == p_sym->p_init->size) break;
        if (p_sym->p_init->basic == type_i32)
            arm_word_output(out_file, (uint32_t) (p_sym->p_init->memory[i].i));
        if (p_sym->p_init->basic == type_f32)
            arm_word_output(out_file, *(uint32_t *) (&p_sym->p_init->memory[i].f));
        i++;
    }
    if ((p_sym->p_type->size - p_sym->p_init->size))
        arm_space_output(out_file, (p_sym->p_type->size - p_sym->p_init->size) * basic_type_get_size(p_sym->p_type->basic));
    fprintf(out_file, "\n");
}