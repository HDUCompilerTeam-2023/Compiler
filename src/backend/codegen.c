#include <program/def.h>

#include <ir/basic_block.h>
#include <ir/bb_param.h>
#include <ir/instr.h>
#include <ir/operand.h>
#include <ir/param.h>
#include <ir_print.h>

#include <ir/vreg.h>
#include <symbol/func.h>
#include <symbol/var.h>

#include <symbol/func.h>
#include <symbol/str.h>
#include <symbol/type.h>
#include <symbol/var.h>

#include <backend/arm/arm_instr_gen.h>
#include <backend/arm/codegen.h>
#include <stdio.h>
static const size_t R_NUM = 16;
// static const size_t S_NUM = 32;
static const size_t REG_NUM = 48;
static const size_t FP = 11;
static const size_t SP = 13;
static const size_t LR = 14;
static const size_t PC = 15;
static const size_t TMP = 14;
// value循环右移bits位
#define ror(value, bits) ((value >> bits) | (value << (sizeof(value) * 8 - bits)))

static const I32CONST_t imm_8_max = 255;
static const I32CONST_t imm_12_max = 4095;
static const I32CONST_t imm_16_max = 65535;

// 是否由八位循环右移偶数位得到
static inline bool if_legal_rotate_imme12(I32CONST_t i32const) {
    if (i32const == 0) return true;
    if (i32const < 0) // 负数
        i32const = -i32const;
    u_int32_t window = ~imm_8_max;
    for (size_t i = 0; i < 16; i++) {
        if (!(window & i32const))
            return true;
        window = ror(window, 2);
    }
    return false;
}

static inline bool if_legal_direct_imme12(I32CONST_t i32const) {
    return !(i32const > imm_12_max || i32const < -imm_12_max);
}

static inline bool if_legal_direct_imme8(I32CONST_t i32const) {
    return !(i32const > imm_8_max || i32const < -imm_8_max);
}

// 对齐到Align的整数倍
static size_t alignTo(size_t N, size_t Align) {
    // (0,Align]返回Align
    return (N + Align - 1) / Align * Align;
}
static inline void stack_alloc(p_arm_codegen_info p_info, p_symbol_func p_func) {
    p_list_head p_node;
    size_t stack_size = 0;
    list_for_each(p_node, &p_func->variable) {
        p_symbol_var p_vmem = list_entry(p_node, symbol_var, node);
        p_info->mem_stack_offset[p_vmem->id] = stack_size;
        printf("vmem %%%ld stack_offset : %ld \n", p_vmem->id, stack_size);
        stack_size += p_vmem->p_type->size;
    }
    p_info->stack_size = alignTo(stack_size, 8);
}

static inline void remap_reg_id(p_symbol_func p_func) {
    size_t r_map[13] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        12, 14 };
    size_t s_map[32] = { 16, 17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
        38, 39, 40, 41, 42, 43, 44, 45, 46, 47 };
    p_list_head p_node;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->if_float)
            p_vreg->reg_id = s_map[p_vreg->reg_id];
        else
            p_vreg->reg_id = r_map[p_vreg->reg_id];
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->reg_id != -1) {
            if (p_vreg->if_float)
                p_vreg->reg_id = s_map[p_vreg->reg_id];
            else
                p_vreg->reg_id = r_map[p_vreg->reg_id];
        }
    }
}

static inline void mov_reg2reg(char *asm_code, size_t rd, size_t rs, bool s) {
    if (rd < R_NUM && rs < R_NUM)
        arm_mov_gen(asm_code, arm_mov, rd, rs, s, 0, false);
    else {
        assert(!s);
        arm_vmov_gen(asm_code, rd, rs);
    }
}

static void swap_reg(p_arm_codegen_info p_info, size_t *r1, size_t *r2, size_t num) {
    size_t *use_reg_count = malloc(sizeof(*use_reg_count) * REG_NUM);
    memset(use_reg_count, 0, REG_NUM * sizeof(*use_reg_count));
    size_t *r2_r1 = malloc(sizeof(*r2_r1) * REG_NUM);
    memset(r2_r1, -1, REG_NUM * sizeof(*r2_r1));
    bool *if_deal = malloc(sizeof(*if_deal) * REG_NUM);
    memset(if_deal, 0, sizeof(*if_deal) * REG_NUM);
    for (size_t i = 0; i < num; i++) {
        assert(r1[i] < REG_NUM && r2[i] < REG_NUM);
        use_reg_count[r1[i]]++;
        r2_r1[r2[i]] = r1[i];
    }
    for (size_t i = 0; i < num; i++) {
        if (if_deal[r2[i]]) continue; // 在之前已经被处理过，但不可能在迭代的过程中发现已经被处理过，因为只有在被使用次数为零时才会处理
        size_t current_work_r1 = r1[i];
        size_t current_work_r2 = r2[i];
        while (use_reg_count[current_work_r2] == 0 && current_work_r1 != current_work_r2) {
            assert((current_work_r1 >= R_NUM && current_work_r2 >= R_NUM) || (current_work_r1 < R_NUM && current_work_r2 < R_NUM));
            mov_reg2reg(p_info->asm_code, current_work_r2, current_work_r1, false);
            use_reg_count[current_work_r1]--;
            if_deal[current_work_r2] = true;
            current_work_r2 = current_work_r1;
            current_work_r1 = r2_r1[current_work_r2];
            if (current_work_r1 == -1)
                break;
        }
    }

    // 现在变成了一一映射
    size_t *current_val_in = malloc(sizeof(*current_val_in) * REG_NUM);
    size_t *reg_val = malloc(sizeof(*reg_val) * REG_NUM);
    memset(current_val_in, 0, sizeof(*current_val_in) * REG_NUM);
    memset(reg_val, 0, sizeof(*reg_val) * REG_NUM);
    for (size_t i = 0; i < num; i++) {
        if (use_reg_count[r1[i]] == 0) continue; // 之前已经被处理过, 处理完后原始reg的使用次数都为1或0
        assert(use_reg_count[r1[i]] == 1);
        current_val_in[r1[i]] = r1[i];
        reg_val[r1[i]] = r1[i];
    }
    for (size_t i = 0; i < num; i++) {
        if (use_reg_count[r1[i]] == 0) continue;
        if (current_val_in[r1[i]] == r2[i]) {
            assert(reg_val[r2[i]] == r1[i]);
            continue;
        }
        // if have tmp
        // if r1[i] >= REG_NUM || r2[i] >= REG_NUM assert(have tmp);
        mov_reg2reg(p_info->asm_code, TMP, r2[i], false);
        mov_reg2reg(p_info->asm_code, r2[i], current_val_in[r1[i]], false);
        mov_reg2reg(p_info->asm_code, current_val_in[r1[i]], TMP, false);
        // else
        // arm_swap_gen(p_info->asm_code, current_val_in[r1[i]], r2[i]);
        // 交换完成后源寄存器放到了正确位置，目标寄存器的值（可能是其他交换的源寄存器）被放到了源寄存器
        current_val_in[reg_val[r2[i]]] = current_val_in[r1[i]];
        reg_val[current_val_in[r1[i]]] = reg_val[r2[i]];
        reg_val[r2[i]] = r1[i];
        current_val_in[r1[i]] = r2[i];
    }

    free(reg_val);
    free(use_reg_count);
    free(current_val_in);
    free(r2_r1);
    free(if_deal);
}

static inline void mov_imm32_gen(char *asm_code, size_t rd, I32CONST_t i32const, bool s) {
    uint32_t low = ((1 << 16) - 1) & (uint32_t) i32const;
    uint32_t high = ((uint32_t) i32const >> 16);
    arm_mov16_gen(asm_code, arm_movw, rd, low, false);
    arm_mov16_gen(asm_code, arm_movt, rd, high, false);
    if (s)
        arm_compare_gen(asm_code, arm_cmp, rd, 0, 0, true);
}

static void mov_int2reg(char *asm_code, size_t rd, I32CONST_t i32const, bool s) {
    // 处理数字范围过大
    if (i32const > imm_16_max) {
        mov_imm32_gen(asm_code, rd, i32const, s);
        return;
    }
    if (i32const < 0) // 负数
    {
        I32CONST_t neg = -i32const - 1;
        if (if_legal_rotate_imme12(neg)) {
            arm_mov_gen(asm_code, arm_mvn, rd, neg, s, 0, true);
            return;
        }
        mov_imm32_gen(asm_code, rd, i32const, s);
        return;
    }
    // movs 和 mov 的立即数范围不同
    if (s && !if_legal_rotate_imme12(i32const)) {
        mov_int2reg(asm_code, rd, i32const, false);
        arm_compare_gen(asm_code, arm_cmp, rd, 0, 0, true);
        return;
    }
    arm_mov_gen(asm_code, arm_mov, rd, i32const, s, 0, true);
}

static void mov_float2reg(p_arm_codegen_info p_info, size_t rd, F32CONST_t floatconst) {
    // 浮点数放到标量寄存器
    assert(rd < R_NUM);
    mov_imm32_gen(p_info->asm_code, rd, *(uint32_t *) (&floatconst), false);
}

static void mov_imme2reg(p_arm_codegen_info p_info, size_t rd, p_ir_operand p_operand, bool s) {
    assert(p_operand->kind == imme);
    if (p_operand->p_type->ref_level > 0) {
        if (p_operand->p_vmem->is_global)
            arm_get_global_addr(p_info->asm_code, rd, p_operand->p_vmem->name);
        else
            arm_data_process_gen(p_info->asm_code, arm_add, rd, FP, p_info->mem_stack_offset[p_operand->p_vmem->id], s, 2, true);
        return;
    }
    switch (p_operand->p_type->basic) {
    case type_i32:
        mov_int2reg(p_info->asm_code, rd, p_operand->i32const, s);
        break;
    case type_f32:
        mov_float2reg(p_info, rd, p_operand->f32const);
        break;
    default:
        assert(0);
        break;
    }
}

static void arm_global_sym_gen(char *asm_code, p_symbol_var p_sym) {
    arm_global_sym_declare_gen(asm_code, p_sym->name, p_sym->p_type->size << 2);
    arm_label_gen(asm_code, p_sym->name);
    if (!p_sym->p_init) {
        arm_space_gen(asm_code, p_sym->p_type->size << 2);
        strcat(asm_code, "\n");
        return;
    }
    size_t i = 0;
    while (i < p_sym->p_init->size) {
        size_t space_loc = i;
        while (i < p_sym->p_init->size && p_sym->p_init->memory[i].f == 0)
            i++;
        size_t space_size = i - space_loc;
        if (space_size)
            arm_space_gen(asm_code, space_size << 2);
        if (i == p_sym->p_init->size) break;
        if (p_sym->p_init->basic == type_i32)
            arm_word_gen(asm_code, (uint32_t) (p_sym->p_init->memory[i].i));
        if (p_sym->p_init->basic == type_f32)
            arm_word_gen(asm_code, *(uint32_t *) (&p_sym->p_init->memory[i].f));
        i++;
    }
    if ((p_sym->p_type->size - p_sym->p_init->size))
        arm_space_gen(asm_code, (p_sym->p_type->size - p_sym->p_init->size) << 2);
    strcat(asm_code, "\n");
}

static void arm_into_func_gen(p_arm_codegen_info p_info, p_symbol_func p_func, size_t stack_size) {
    arm_func_sym_declare_gen(p_info->asm_code, p_func->name);
    arm_label_gen(p_info->asm_code, p_func->name);
    size_t r[2] = { FP, LR };
    arm_push_gen(p_info->asm_code, r, 2);
    if (!if_legal_rotate_imme12(stack_size)) {
        mov_int2reg(p_info->asm_code, 12, stack_size, false);
        arm_data_process_gen(p_info->asm_code, arm_sub, SP, SP, 12, false, 2, false);
    }
    else
        arm_data_process_gen(p_info->asm_code, arm_sub, SP, SP, stack_size, false, 2, true);
    arm_mov_gen(p_info->asm_code, arm_mov, FP, SP, false, 0, false);
}

static void arm_out_func_gen(p_arm_codegen_info p_info, size_t stack_size) {
    if (!if_legal_rotate_imme12(stack_size)) {
        mov_int2reg(p_info->asm_code, 12, stack_size, false);
        arm_data_process_gen(p_info->asm_code, arm_add, FP, FP, 12, false, 2, false);
    }
    else
        arm_data_process_gen(p_info->asm_code, arm_add, FP, FP, stack_size, false, 2, true);
    arm_mov_gen(p_info->asm_code, arm_mov, SP, FP, false, 0, false);
    size_t r[2] = { FP, PC };
    arm_pop_gen(p_info->asm_code, r, 2);
    arm_jump_reg_gen(p_info->asm_code, arm_bx, arm_al, LR);
}

static inline char *get_block_label(char *func_name, size_t block_id) {
    char *str_id = uint32_str(block_id);
    char *extra = malloc(strlen(func_name) + strlen(str_id) + 4);
    strcpy(extra, ".");
    strcat(extra, func_name);
    strcat(extra, "_b");
    strcat(extra, str_id);
    free(str_id);
    return extra;
}
static inline void arm_block_label_gen(char *asm_code, char *func_name, size_t block_id) {
    char *tmp = get_block_label(func_name, block_id);
    arm_label_gen(asm_code, tmp);
    free(tmp);
}
static inline void arm_block_jump_label_gen(char *asm_code, arm_instr_type instr_type, arm_cond_type cond_type, char *func_name, size_t block_id) {
    char *tmp = get_block_label(func_name, block_id);
    arm_jump_label_gen(asm_code, instr_type, cond_type, tmp);
    free(tmp);
}

void swap_in_call(p_arm_codegen_info p_info, p_ir_param_list p_param_list) {
    p_list_head p_node;
    size_t *src_reg = malloc(sizeof(*src_reg) * REG_NUM);
    size_t *des_reg = malloc(sizeof(*des_reg) * REG_NUM);
    p_ir_operand *src_immes = malloc(sizeof(void *) * REG_NUM);
    size_t *des_imme_reg = malloc(sizeof(*des_imme_reg) * REG_NUM);

    size_t num = 0;
    size_t i = 0;
    size_t r = 0;
    size_t s = R_NUM;
    list_for_each(p_node, &p_param_list->param) {
        p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
        if (p_param->kind == imme) {
            src_immes[i] = p_param;
            if (p_param->p_type->ref_level > 0 || p_param->p_type->basic == type_i32)
                des_imme_reg[i] = r++;
            else if (p_param->p_type->basic == type_f32)
                des_imme_reg[i] = s++;
            i++;
            continue;
        }
        src_reg[num] = p_param->p_vreg->reg_id;
        if (p_param->p_vreg->if_float)
            des_reg[num] = s++;
        else
            des_reg[num] = r++;
        num++;
    }
    // TO DO: param in stack
    swap_reg(p_info, src_reg, des_reg, num);
    for (size_t j = 0; j < i; j++)
        mov_imme2reg(p_info, des_imme_reg[j], src_immes[j], false);
    free(src_reg);
    free(des_reg);
    free(des_imme_reg);
    free(src_immes);
}

static void swap_in_phi(p_arm_codegen_info p_info, p_ir_bb_phi_list p_bb_phi_list, p_ir_bb_param_list p_bb_param_list) {
    size_t *src_reg = malloc(sizeof(*src_reg) * REG_NUM);
    size_t *des_reg = malloc(sizeof(*des_reg) * REG_NUM);
    size_t num = 0;
    p_list_head p_node1, p_node2;
    for (p_node1 = p_bb_param_list->bb_param.p_next, p_node2 = p_bb_phi_list->bb_phi.p_next; p_node1 != &p_bb_param_list->bb_param
         && p_node2 != &p_bb_phi_list->bb_phi;
         p_node1 = p_node1->p_next, p_node2 = p_node2->p_next) {
        p_ir_operand p_param = list_entry(p_node1, ir_bb_param, node)->p_bb_param;
        p_ir_vreg p_phi = list_entry(p_node2, ir_bb_phi, node)->p_bb_phi;
        assert(p_param);
        if (p_param->kind == imme) {
            continue;
        }
        // if (p_phi->p_bb_phi->kind == imme) continue;
        src_reg[num] = p_param->p_vreg->reg_id;
        des_reg[num] = p_phi->reg_id;
        num++;
    }
    swap_reg(p_info, src_reg, des_reg, num);
    for (p_node1 = &p_bb_param_list->bb_param, p_node2 = &p_bb_phi_list->bb_phi; p_node1 != &p_bb_param_list->bb_param
         && p_node2 != &p_bb_param_list->bb_param;
         p_node1 = p_node1->p_next, p_node2 = p_node2->p_next) {
        // TODO param in mem
        p_ir_operand p_param = list_entry(p_node1, ir_bb_param, node)->p_bb_param;
        p_ir_vreg p_phi = list_entry(p_node2, ir_bb_phi, node)->p_bb_phi;
        if (p_param->kind == imme)
            mov_imme2reg(p_info, p_phi->reg_id, p_param, false);
    }
    free(src_reg);
    free(des_reg);
}

static void arm_unary_instr_codegen(p_arm_codegen_info p_info, p_ir_unary_instr p_unary_instr) {
    size_t rd = p_unary_instr->p_des->reg_id;
    bool s = p_unary_instr->p_des->if_cond;
    char *asm_code = p_info->asm_code;

    switch (p_unary_instr->op) {
    case ir_minus_op:
        assert(p_unary_instr->p_src->kind == reg);
        if (rd >= R_NUM)
            arm_vneg_gen(asm_code, rd, p_unary_instr->p_src->p_vreg->reg_id);
        else
            arm_data_process_gen(asm_code, arm_rsb, rd, p_unary_instr->p_src->p_vreg->reg_id, 0, s, 0, true);
        break;
    case ir_val_assign:
        if (p_unary_instr->p_src->kind == imme) {
            mov_imme2reg(p_info, rd, p_unary_instr->p_src, s);
            break;
        }
        mov_reg2reg(asm_code, rd, p_unary_instr->p_src->p_vreg->reg_id, s);
        break;
    case ir_i2f_op:
        arm_vcvt_gen(asm_code, arm_int2float, rd, p_unary_instr->p_src->p_vreg->reg_id);
        break;
    case ir_f2i_op:
        arm_vcvt_gen(asm_code, arm_float2int, rd, p_unary_instr->p_src->p_vreg->reg_id);
        break;
    default:
        assert(0);
        break;
    }
}
static void arm_binary_instr_codegen(p_arm_codegen_info p_info, p_ir_binary_instr p_binary_instr) {
    size_t rd = p_binary_instr->p_des->reg_id;
    bool s = p_binary_instr->p_des->if_cond;
    char *asm_code = p_info->asm_code;
    switch (p_binary_instr->op) {
    case ir_add_op:
        if (p_binary_instr->p_src1->kind == imme && p_binary_instr->p_src2->kind == imme) {
            size_t stack_offset = p_info->mem_stack_offset[p_binary_instr->p_src1->p_vmem->id];
            if (p_binary_instr->p_src2->p_type->ref_level > 0)
                stack_offset += (p_binary_instr->p_src1->i32const);
            else
                stack_offset += (p_binary_instr->p_src2->i32const);
            if (!if_legal_rotate_imme12(stack_offset)) {
                mov_int2reg(asm_code, rd, stack_offset << 2, false);
                arm_data_process_gen(asm_code, arm_add, rd, FP, rd, s, 0, false);
            }
            else
                arm_data_process_gen(asm_code, arm_add, rd, FP, stack_offset, s, 2, true);
            break;
        }
        // a pointer in reg, need lsl
        if (p_binary_instr->p_src1->kind == imme) {
            if (p_binary_instr->p_src1->p_type->ref_level > 0) {
                size_t offset = p_info->mem_stack_offset[p_binary_instr->p_src1->p_vmem->id];
                arm_data_process_gen(asm_code, arm_add, rd, p_binary_instr->p_src2->p_vreg->reg_id, offset, false, 0, true);
                arm_data_process_gen(asm_code, arm_add, rd, FP, rd, s, 2, false);
                break;
            }
            assert(p_binary_instr->p_src1->p_type->basic == type_i32);
            size_t lsl_imme = 0;
            if (p_binary_instr->p_src2->p_type->ref_level) lsl_imme = 2;
            arm_data_process_gen(asm_code, arm_add, rd, p_binary_instr->p_src2->p_vreg->reg_id, p_binary_instr->p_src1->i32const, s, lsl_imme, true);
            break;
        }
        if (p_binary_instr->p_src2->kind == imme) {
            if (p_binary_instr->p_src2->p_type->ref_level > 0) {
                size_t offset = p_info->mem_stack_offset[p_binary_instr->p_src2->p_vmem->id];
                arm_data_process_gen(asm_code, arm_add, rd, p_binary_instr->p_src1->p_vreg->reg_id, offset, false, 0, true);
                arm_data_process_gen(asm_code, arm_add, rd, FP, rd, s, 2, false);
                break;
            }
            assert(p_binary_instr->p_src2->p_type->basic == type_i32);
            size_t lsl_imme = 0;
            if (p_binary_instr->p_src1->p_type->ref_level) lsl_imme = 2;
            arm_data_process_gen(asm_code, arm_add, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->i32const, s, lsl_imme, true);
            break;
        }
        if (p_binary_instr->p_src1->p_type->ref_level) {
            arm_data_process_gen(asm_code, arm_add, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id, s, 2, false);
            break;
        }
        if (p_binary_instr->p_src2->p_type->ref_level) {
            arm_data_process_gen(asm_code, arm_add, rd, p_binary_instr->p_src2->p_vreg->reg_id, p_binary_instr->p_src1->p_vreg->reg_id, s, 2, false);
            break;
        }
        if (rd >= R_NUM)
            arm_vdata_process_gen(asm_code, arm_add, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id);
        else
            arm_data_process_gen(asm_code, arm_add, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id, s, 0, false);
        break;
    case ir_sub_op:
        if (p_binary_instr->p_src1->kind == imme && p_binary_instr->p_src2->kind == imme) {
            assert(p_binary_instr->p_src1->p_type->ref_level > 0);
            size_t stack_offset = p_info->mem_stack_offset[p_binary_instr->p_src1->p_vmem->id] - p_binary_instr->p_src2->i32const;
            if (!if_legal_rotate_imme12(stack_offset)) {
                mov_int2reg(asm_code, rd, stack_offset << 2, false);
                arm_data_process_gen(asm_code, arm_add, rd, FP, rd, s, 0, false);
            }
            else
                arm_data_process_gen(asm_code, arm_add, rd, FP, stack_offset, s, 2, true);
            break;
        }
        // a pointer in reg, need lsl
        assert(p_binary_instr->p_src2->p_type->ref_level == 0);
        if (p_binary_instr->p_src1->kind == imme) {
            if (p_binary_instr->p_src1->p_type->ref_level > 0) {
                size_t offset = p_info->mem_stack_offset[p_binary_instr->p_src1->p_vmem->id];
                arm_data_process_gen(asm_code, arm_rsb, rd, p_binary_instr->p_src2->p_vreg->reg_id, offset, false, 0, true);
                arm_data_process_gen(asm_code, arm_add, rd, FP, rd, s, 2, false);
                break;
            }
            assert(p_binary_instr->p_src1->p_type->basic == type_i32);
            arm_data_process_gen(asm_code, arm_rsb, rd, p_binary_instr->p_src2->p_vreg->reg_id, p_binary_instr->p_src1->i32const, s, 0, true);
            break;
        }
        if (p_binary_instr->p_src2->kind == imme) {
            assert(p_binary_instr->p_src1->p_type->ref_level == 0);
            assert(p_binary_instr->p_src2->p_type->basic == type_i32);
            size_t lsl_imme = 0;
            if (p_binary_instr->p_src1->p_type->ref_level) lsl_imme = 2;
            arm_data_process_gen(asm_code, arm_sub, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->i32const, s, lsl_imme, true);
            break;
        }
        if (p_binary_instr->p_src1->p_type->ref_level) {
            arm_data_process_gen(asm_code, arm_sub, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id, s, 2, false);
            break;
        }
        if (rd >= R_NUM)
            arm_vdata_process_gen(asm_code, arm_sub, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id);
        else
            arm_data_process_gen(asm_code, arm_sub, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id, s, 0, false);
        break;
    case ir_mul_op:
        if (rd >= R_NUM)
            arm_vdata_process_gen(asm_code, arm_mul, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id);
        else
            arm_mul_gen(asm_code, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id, s);
        break;
    case ir_div_op:
        assert(!s);
        if (rd >= R_NUM)
            arm_vdata_process_gen(asm_code, arm_div, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id);
        else
            arm_sdiv_gen(asm_code, rd, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id);
        break;
    case ir_eq_op:
    case ir_neq_op:
    case ir_l_op:
    case ir_leq_op:
    case ir_g_op:
    case ir_geq_op:
        // can pointer compare ?
        assert(p_binary_instr->p_src1->kind == reg);
        if (p_binary_instr->p_src2->kind == imme) {
            assert(p_binary_instr->p_src2->p_type->basic == type_i32);
            if (p_binary_instr->p_src2->i32const < 0)
                arm_compare_gen(asm_code, arm_cmn, p_binary_instr->p_src1->p_vreg->reg_id, -p_binary_instr->p_src2->i32const, 0, true);
            else
                arm_compare_gen(asm_code, arm_cmp, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->i32const, 0, true);
        }
        else {
            if (p_binary_instr->p_src1->p_vreg->reg_id >= R_NUM)
                arm_vcompare_gen(asm_code, arm_cmp, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id);
            else
                arm_compare_gen(asm_code, arm_cmp, p_binary_instr->p_src1->p_vreg->reg_id, p_binary_instr->p_src2->p_vreg->reg_id, 0, false);
        }
        if (!s) {
            arm_movcond_gen(asm_code, arm_eq, rd, 1, 0, true);
            arm_movcond_gen(asm_code, arm_ne, rd, 0, 0, true);
        }
        break;
    case ir_and_op:
    case ir_or_op:
    case ir_mod_op:
        assert(0);
    }
}

static void arm_call_instr_codegen(p_arm_codegen_info p_info, p_ir_instr p_instr) {
    // store var in reg
    p_ir_call_instr p_call_instr = &p_instr->ir_call;
    size_t rs = 0;
    if (p_call_instr->p_des && p_call_instr->p_func->ret_type == type_f32)
        rs = R_NUM;
    swap_in_call(p_info, p_call_instr->p_param_list);
    arm_jump_label_gen(p_info->asm_code, arm_bl, arm_al, p_call_instr->p_func->name);
    if (p_call_instr->p_des) {
        if (rs != p_call_instr->p_des->reg_id)
            mov_reg2reg(p_info->asm_code, p_call_instr->p_des->reg_id, rs, false);
    }
}

static void arm_store_instr_gen(p_arm_codegen_info p_info, p_ir_store_instr p_store_instr) {
    assert(p_store_instr->p_src->kind == reg);
    size_t rd = p_store_instr->p_src->p_vreg->reg_id;
    char *asm_code = p_info->asm_code;
    if (p_store_instr->p_addr->kind == imme) {
        assert(p_store_instr->p_addr->p_type->ref_level);
        size_t stack_offset = p_info->mem_stack_offset[p_store_instr->p_addr->p_vmem->id];
        if (p_store_instr->p_offset) {
            assert(p_store_instr->p_offset->kind == imme);
            stack_offset += p_store_instr->p_offset->i32const;
        }
        if (rd >= R_NUM) {
            if (!if_legal_direct_imme8(stack_offset)) {
                arm_mov_gen(asm_code, arm_mov, TMP, FP, false, 0, false);
                while (!if_legal_direct_imme8(stack_offset)) {
                    arm_data_process_gen(asm_code, arm_add, TMP, TMP, imm_8_max, false, 2, true);
                    stack_offset -= imm_8_max;
                }
                arm_vstore_gen(asm_code, rd, TMP, stack_offset << 2, true);
            }
            else
                arm_vstore_gen(asm_code, rd, FP, stack_offset << 2, true);
            return;
        }
        if (!if_legal_direct_imme12(stack_offset << 2)) { // 这里有bug,要用到临时寄存器,待解决
            mov_int2reg(asm_code, TMP, stack_offset << 2, false);
            arm_store_gen(asm_code, arm_store, rd, FP, TMP, 0, false);
        }
        else
            arm_store_gen(asm_code, arm_store, rd, FP, stack_offset, 2, true);
        return;
    }
    size_t rn = p_store_instr->p_addr->p_vreg->reg_id;
    if (rd >= R_NUM) {
        if (p_store_instr->p_offset) // 这边为了保证vldr 的 offset 是合法的，应该在 arm_lir 的imme2reg添加对应处理，待解决
            if (p_store_instr->p_offset->kind == imme) {
                size_t stack_offset = p_store_instr->p_offset->i32const;
                if (!if_legal_direct_imme8(stack_offset)) {
                    arm_mov_gen(asm_code, arm_mov, TMP, rn, false, 0, false);
                    while (!if_legal_direct_imme8(stack_offset)) {
                        arm_data_process_gen(asm_code, arm_add, TMP, TMP, imm_8_max, false, 2, true);
                        stack_offset -= imm_8_max;
                    }
                    arm_vstore_gen(asm_code, rd, TMP, stack_offset << 2, true);
                }
                else
                    arm_vstore_gen(asm_code, rd, rn, stack_offset << 2, false);
            }
            else {
                arm_data_process_gen(asm_code, arm_lsl, TMP, p_store_instr->p_offset->p_vreg->reg_id, 2, false, 0, true);
                arm_vstore_gen(asm_code, rd, rn, TMP, false);
            }
        else
            arm_vstore_gen(asm_code, rd, rn, 0, true);
        return;
    }
    if (p_store_instr->p_offset)
        if (p_store_instr->p_offset->kind == imme)
            arm_store_gen(asm_code, arm_store, rd, rn, p_store_instr->p_offset->i32const, 2, true);
        else
            arm_store_gen(asm_code, arm_store, rd, rn, p_store_instr->p_offset->p_vreg->reg_id, 2, false);
    else
        arm_store_gen(asm_code, arm_store, rd, rn, 0, 0, true);
}

static void arm_load_instr_gen(p_arm_codegen_info p_info, p_ir_load_instr p_load_instr) {
    size_t rd = p_load_instr->p_des->reg_id;
    char *asm_code = p_info->asm_code;
    if (p_load_instr->p_addr->kind == imme) {
        assert(p_load_instr->p_addr->p_type->ref_level);
        size_t stack_offset = p_info->mem_stack_offset[p_load_instr->p_addr->p_vmem->id];
        if (p_load_instr->p_offset) {
            assert(p_load_instr->p_offset->kind == imme);
            stack_offset += p_load_instr->p_offset->i32const;
            return;
        }
        if (rd >= R_NUM) {
            if (!if_legal_direct_imme8(stack_offset)) {
                arm_mov_gen(asm_code, arm_mov, TMP, FP, false, 0, false);
                while (!if_legal_direct_imme8(stack_offset)) {
                    arm_data_process_gen(asm_code, arm_add, TMP, TMP, imm_8_max, false, 2, true);
                    stack_offset -= imm_8_max;
                }
                arm_vload_gen(asm_code, rd, TMP, stack_offset << 2, true);
            }
            else
                arm_vload_gen(asm_code, rd, FP, stack_offset << 2, 2);
            return;
        }
        if (!if_legal_direct_imme12(stack_offset << 2)) {
            mov_int2reg(asm_code, rd, stack_offset << 2, false);
            arm_load_gen(asm_code, arm_load, rd, FP, rd, 0, false);
        }
        else
            arm_load_gen(asm_code, arm_load, rd, FP, stack_offset, 2, true);
        return;
    }
    size_t rn = p_load_instr->p_addr->p_vreg->reg_id;
    if (rd >= R_NUM) {
        if (p_load_instr->p_offset)
            if (p_load_instr->p_offset->kind == imme) {
                size_t stack_offset = p_load_instr->p_offset->i32const;
                if (!if_legal_direct_imme8(stack_offset)) {
                    arm_mov_gen(asm_code, arm_mov, TMP, rn, false, 0, false);
                    while (!if_legal_direct_imme8(stack_offset)) {
                        arm_data_process_gen(asm_code, arm_add, TMP, TMP, imm_8_max, false, 2, true);
                        stack_offset -= imm_8_max;
                    }
                    arm_vload_gen(asm_code, rd, TMP, stack_offset << 2, true);
                }
                else
                    arm_vload_gen(asm_code, rd, rn, stack_offset << 2, true);
            }
            else {
                arm_data_process_gen(asm_code, arm_lsl, TMP, p_load_instr->p_offset->p_vreg->reg_id, 2, false, 0, true);
                arm_vload_gen(asm_code, rd, rn, TMP, false);
            }
        else
            arm_vload_gen(asm_code, rd, rn, 0, true);
        return;
    }
    if (p_load_instr->p_offset)
        if (p_load_instr->p_offset->kind == imme)
            arm_load_gen(asm_code, arm_load, rd, rn, p_load_instr->p_offset->i32const, 2, true);
        else
            arm_load_gen(asm_code, arm_load, rd, rn, p_load_instr->p_offset->p_vreg->reg_id, 2, false);
    else
        arm_load_gen(asm_code, arm_load, rd, rn, 0, 0, true);
}
static void arm_instr_codegen(p_arm_codegen_info p_info, p_ir_instr p_instr) {
    switch (p_instr->irkind) {
    case ir_binary:
        arm_binary_instr_codegen(p_info, &p_instr->ir_binary);
        break;
    case ir_unary:
        arm_unary_instr_codegen(p_info, &p_instr->ir_unary);
        break;
    case ir_call:
        arm_call_instr_codegen(p_info, p_instr);
        break;
    case ir_store:
        arm_store_instr_gen(p_info, &p_instr->ir_store);
        break;
    case ir_load:
        arm_load_instr_gen(p_info, &p_instr->ir_load);
        break;
    case ir_gep:
        assert(0);
    }
}

static arm_cond_type get_jump_type(p_arm_codegen_info p_info, p_ir_vreg p_vreg) {
    assert(p_vreg->if_cond);
    assert(!p_vreg->is_bb_param);
    if (p_vreg->p_instr_def->irkind != ir_binary)
        return arm_ne;
    switch (p_vreg->p_instr_def->ir_binary.op) {
    case ir_eq_op:
        return arm_eq;
    case ir_neq_op:
        return arm_ne;
    case ir_l_op:
        return arm_lt;
    case ir_leq_op:
        return arm_le;
    case ir_g_op:
        return arm_gt;
    case ir_geq_op:
        return arm_ge;
    default:
        return arm_ne;
    }
}

static void arm_basic_block_codegen(p_arm_codegen_info p_info, p_ir_basic_block p_block, p_ir_basic_block p_next_block, p_symbol_func p_func) {
    arm_block_label_gen(p_info->asm_code, p_func->name, p_block->block_id);
    p_list_head p_node;
    list_for_each(p_node, &p_block->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        arm_instr_codegen(p_info, p_instr);
    }
    switch (p_block->p_branch->kind) {
    case ir_br_branch:
        swap_in_phi(p_info, p_block->p_branch->p_target_1->p_block->basic_block_phis, p_block->p_branch->p_target_1->p_block_param);
        if (p_block->p_branch->p_target_1->p_block != p_next_block)
            arm_block_jump_label_gen(p_info->asm_code, arm_b, arm_al, p_func->name, p_block->p_branch->p_target_1->p_block->block_id);
        break;
    case ir_cond_branch:
        assert(p_block->p_branch->p_exp->kind == reg);
        assert(list_head_alone(&p_block->p_branch->p_target_1->p_block_param->bb_param));
        assert(list_head_alone(&p_block->p_branch->p_target_2->p_block_param->bb_param));
        arm_cond_type type = get_jump_type(p_info, p_block->p_branch->p_exp->p_vreg);
        swap_in_phi(p_info, p_block->p_branch->p_target_1->p_block->basic_block_phis, p_block->p_branch->p_target_1->p_block_param);
        arm_block_jump_label_gen(p_info->asm_code, arm_b, type, p_func->name, p_block->p_branch->p_target_1->p_block->block_id);
        swap_in_phi(p_info, p_block->p_branch->p_target_2->p_block->basic_block_phis, p_block->p_branch->p_target_2->p_block_param);
        if (p_block->p_branch->p_target_2->p_block != p_next_block)
            arm_block_jump_label_gen(p_info->asm_code, arm_b, arm_al, p_func->name, p_block->p_branch->p_target_2->p_block->block_id);
        break;
    case ir_ret_branch:
        if (!p_block->p_branch->p_exp) {
            arm_out_func_gen(p_info, p_info->stack_size);
            break;
        }
        size_t rd = 0;
        if (p_block->p_branch->p_exp->p_type->basic == type_f32)
            rd = R_NUM;
        if (p_block->p_branch->p_exp->kind == imme)
            mov_imme2reg(p_info, rd, p_block->p_branch->p_exp, false);
        else if (rd != p_block->p_branch->p_exp->p_vreg->reg_id)
            mov_reg2reg(p_info->asm_code, rd, p_block->p_branch->p_exp->p_vreg->reg_id, false);
        arm_out_func_gen(p_info, p_info->stack_size);
        break;
    case ir_abort_branch:
        break;
    }
}

static p_arm_codegen_info arm_codegen_info_gen(p_symbol_func p_func, char *asm_code) {
    p_arm_codegen_info p_info = malloc(sizeof(*p_info));
    p_info->mem_stack_offset = malloc(p_func->var_cnt * sizeof(*p_info->mem_stack_offset));
    p_info->asm_code = asm_code;
    p_info->p_func = p_func;
    return p_info;
}
static void arm_codegen_info_drop(p_arm_codegen_info p_info) {
    free(p_info->mem_stack_offset);
    free(p_info);
}

void arm_func_codegen(p_symbol_func p_func, char *asm_code) {
    if (list_head_alone(&p_func->block)) return;
    p_arm_codegen_info p_info = arm_codegen_info_gen(p_func, asm_code);

    stack_alloc(p_info, p_func);
    remap_reg_id(p_func);

    arm_into_func_gen(p_info, p_func, p_info->stack_size);

    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_ir_basic_block p_next_block = list_entry(p_block_node->p_next, ir_basic_block, node);
        arm_basic_block_codegen(p_info, p_basic_block, p_next_block, p_func);
    }
    arm_codegen_info_drop(p_info);
}

void arm_codegen_pass(p_program p_ir, char *asm_code) {
    strcat(asm_code, "   .arch armv7ve\n");
    strcat(asm_code, "   .arm\n");
    strcat(asm_code, "   .fpu neon-vfpv4\n");

    p_list_head p_node;
    list_for_each(p_node, &p_ir->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        assert(p_var->is_global);
        arm_global_sym_gen(asm_code, p_var);
    }

    strcat(asm_code, "\n");
    strcat(asm_code, ".section .text\n");
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        arm_func_codegen(p_func, asm_code);
    }
}
