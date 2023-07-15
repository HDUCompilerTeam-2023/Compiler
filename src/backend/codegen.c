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
#include <ir_opt/lir_gen/arm_standard.h>
#include <stdio.h>

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
            mov_reg2reg(p_info->asm_code, current_work_r2, current_work_r1, false);
            p_info->mov_num++;
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
        p_info->swap_num++;
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
        assert(p_operand->p_vmem->is_global);
        arm_get_global_addr(p_info->asm_code, rd, p_operand->p_vmem->name);
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

static inline arm_cond_type get_cond_type(ir_binary_op op) {
    switch (op) {
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

static inline arm_cond_type get_opposite_type(arm_cond_type type) {
    switch (type) {
    case arm_eq:
        return arm_ne;
    case arm_ne:
        return arm_eq;
    case arm_ge:
        return arm_lt;
    case arm_gt:
        return arm_le;
    case arm_le:
        return arm_gt;
    case arm_lt:
        return arm_ge;
    case arm_al:
        return arm_al;
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
    p_info->save_reg_r[p_info->save_reg_r_num] = LR;
    arm_push_gen(p_info->asm_code, p_info->save_reg_r, p_info->save_reg_r_num + 1);
    arm_vpush_gen(p_info->asm_code, p_info->save_reg_s, p_info->save_reg_s_num);
    if (!if_legal_rotate_imme12(stack_size)) {
        mov_int2reg(p_info->asm_code, 12, stack_size, false);
        arm_data_process_gen(p_info->asm_code, arm_sub, SP, SP, 12, false, 0, false);
    }
    else
        arm_data_process_gen(p_info->asm_code, arm_sub, SP, SP, stack_size, false, 0, true);
}

static void arm_out_func_gen(p_arm_codegen_info p_info, size_t stack_size) {
    if (!if_legal_rotate_imme12(stack_size)) {
        mov_int2reg(p_info->asm_code, TMP, stack_size, false);
        arm_data_process_gen(p_info->asm_code, arm_add, SP, SP, TMP, false, 0, false);
    }
    else
        arm_data_process_gen(p_info->asm_code, arm_add, SP, SP, stack_size, false, 0, true);
    p_info->save_reg_r[p_info->save_reg_r_num] = PC;
    arm_pop_gen(p_info->asm_code, p_info->save_reg_r, p_info->save_reg_r_num + 1);
    arm_vpop_gen(p_info->asm_code, p_info->save_reg_s, p_info->save_reg_s_num);
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
    p_ir_param *src_immes = malloc(sizeof(void *) * REG_NUM);
    size_t *des_imme_reg = malloc(sizeof(*des_imme_reg) * REG_NUM);

    size_t num = 0;
    size_t i = 0;
    size_t r = 0;
    size_t s = 0;
    list_for_each(p_node, &p_param_list->param) {
        p_ir_param p_param_node = list_entry(p_node, ir_param, node);
        if (p_param_node->is_in_mem) continue;
        p_ir_operand p_param = p_param_node->p_param;
        if (p_param->kind == imme) {
            src_immes[i] = p_param_node;
            if (if_in_r(p_param->p_type))
                des_imme_reg[i] = caller_save_reg_r[r++];
            else if (p_param->p_type->basic == type_f32)
                des_imme_reg[i] = caller_save_reg_s[s++];
            i++;
            continue;
        }
        src_reg[num] = p_param->p_vreg->reg_id;
        if (if_in_r(p_param->p_type))
            des_reg[num] = caller_save_reg_r[r++];
        else
            des_reg[num] = caller_save_reg_s[s++];

        if (p_param_node->is_stack_ptr) {
            arm_data_process_gen(p_info->asm_code, arm_add, src_reg[num], SP, src_reg[num], false, 0, false);
        }
        num++;
    }
    // TO DO: param in stack
    swap_reg(p_info, src_reg, des_reg, num);
    for (size_t j = 0; j < i; j++) {
        if (src_immes[j]->is_stack_ptr) {
            assert(src_immes[i]->p_param->kind == imme);
            arm_data_process_gen(p_info->asm_code, arm_add, des_imme_reg[j], SP, src_immes[j]->p_param->i32const, false, 0, true);
            continue;
        }
        mov_imme2reg(p_info, des_imme_reg[j], src_immes[j]->p_param, false);
    }
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

static inline void swap_in_func_param(p_arm_codegen_info p_info, p_symbol_func p_func) {
    size_t *src_reg = malloc(sizeof(*src_reg) * REG_NUM);
    size_t *des_reg = malloc(sizeof(*des_reg) * REG_NUM);
    p_list_head p_node;
    size_t r = 0;
    size_t s = 0;
    size_t num = 0;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_param = list_entry(p_node, ir_vreg, node);
        if (if_in_r(p_param->p_type))
            src_reg[num] = caller_save_reg_r[r++];
        else
            src_reg[num] = caller_save_reg_s[s++];
        des_reg[num] = p_param->reg_id;
        num++;
    }
    swap_reg(p_info, src_reg, des_reg, num);
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
    p_ir_vreg p_des = p_binary_instr->p_des;
    bool s = p_binary_instr->p_des->if_cond;
    p_ir_operand p_src1 = p_binary_instr->p_src1;
    p_ir_operand p_src2 = p_binary_instr->p_src2;
    size_t rd = p_des->reg_id;
    char *asm_code = p_info->asm_code;
    switch (p_binary_instr->op) {
    case ir_add_op:
        if (p_src1->kind == imme) {
            assert(p_src2->kind == reg);
            assert(p_src1->p_type->ref_level == 0 && p_src1->p_type->basic == type_i32);
            arm_data_process_gen(asm_code, arm_add, p_des->reg_id, p_src2->p_vreg->reg_id, p_src1->i32const, s, 0, true);
            break;
        }
        if (p_src2->kind == imme) {
            assert(p_src1->kind == reg);
            assert(p_src2->p_type->ref_level == 0 && p_src2->p_type->basic == type_i32);
            arm_data_process_gen(asm_code, arm_add, p_des->reg_id, p_src1->p_vreg->reg_id, p_src2->i32const, s, 0, true);
            break;
        }
        if (p_des->if_float) {
            arm_vdata_process_gen(asm_code, arm_add, p_des->reg_id, p_src1->p_vreg->reg_id, p_src2->p_vreg->reg_id);
            break;
        }
        arm_data_process_gen(asm_code, arm_add, p_des->reg_id, p_src1->p_vreg->reg_id, p_src2->p_vreg->reg_id, s, 0, false);
        break;
    case ir_sub_op:
        if (p_src1->kind == imme) {
            assert(p_src2->kind == reg);
            assert(p_src1->p_type->ref_level == 0 && p_src1->p_type->basic == type_i32);
            arm_data_process_gen(asm_code, arm_rsb, p_des->reg_id, p_src2->p_vreg->reg_id, p_src1->i32const, s, 0, true);
            break;
        }
        if (p_src2->kind == imme) {
            assert(p_src1->kind == reg);
            assert(p_src2->p_type->ref_level == 0 && p_src2->p_type->basic == type_i32);
            arm_data_process_gen(asm_code, arm_sub, p_des->reg_id, p_src1->p_vreg->reg_id, p_src2->i32const, s, 0, true);
            break;
        }
        if (p_des->if_float) {
            arm_vdata_process_gen(asm_code, arm_sub, p_des->reg_id, p_src1->p_vreg->reg_id, p_src2->p_vreg->reg_id);
            break;
        }
        arm_data_process_gen(asm_code, arm_sub, p_des->reg_id, p_src1->p_vreg->reg_id, p_src2->p_vreg->reg_id, s, 0, false);
        break;
    case ir_mul_op:
        if (p_des->if_float)
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
            arm_cond_type type = get_cond_type(p_binary_instr->op);
            arm_movcond_gen(asm_code, type, rd, 1, 0, true);
            arm_movcond_gen(asm_code, get_opposite_type(type), rd, 0, 0, true);
        }
        break;
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
    p_ir_vreg p_src = p_store_instr->p_src->p_vreg;
    p_ir_operand p_addr = p_store_instr->p_addr;
    char *asm_code = p_info->asm_code;
    if (p_store_instr->p_call_param && p_store_instr->p_call_param->is_stack_ptr)
        arm_data_process_gen(asm_code, arm_add, p_src->reg_id, SP, p_src->reg_id, false, 0, false);
    if (p_store_instr->is_stack_ptr) {
        if (p_src->if_float) {
            if (p_addr->kind == imme)
                arm_vstore_gen(asm_code, p_src->reg_id, SP, p_addr->i32const, true);
            else {
                arm_data_process_gen(asm_code, arm_add, TMP, SP, p_addr->p_vreg->reg_id, false, 0, false);
                arm_vstore_gen(asm_code, p_src->reg_id, TMP, 0, true);
            }
        }
        else {
            if (p_addr->kind == imme)
                arm_store_gen(asm_code, arm_store, p_src->reg_id, SP, p_addr->i32const, 0, true);
            else
                arm_store_gen(asm_code, arm_store, p_src->reg_id, SP, p_addr->p_vreg->reg_id, 0, false);
        }
    }
    else {
        assert(p_addr->kind == reg);
        if (p_src->if_float)
            arm_vstore_gen(asm_code, p_src->reg_id, p_addr->p_vreg->reg_id, 0, true);
        else
            arm_store_gen(asm_code, arm_store, p_src->reg_id, p_addr->p_vreg->reg_id, 0, 0, true);
    }
}

static void arm_load_instr_gen(p_arm_codegen_info p_info, p_ir_load_instr p_load_instr) {
    p_ir_vreg p_des = p_load_instr->p_des;
    char *asm_code = p_info->asm_code;
    p_ir_operand p_addr = p_load_instr->p_addr;
    if (p_load_instr->is_stack_ptr) {
        if (p_des->if_float) {
            if (p_addr->kind == imme)
                arm_vload_gen(asm_code, p_des->reg_id, SP, p_addr->i32const, true);
            else {
                arm_data_process_gen(asm_code, arm_add, TMP, SP, p_addr->p_vreg->reg_id, false, 0, false);
                arm_vload_gen(asm_code, p_des->reg_id, TMP, 0, true);
            }
        }
        else {
            if (p_addr->kind == imme)
                arm_load_gen(asm_code, arm_load, p_des->reg_id, SP, p_addr->i32const, 0, true);
            else
                arm_load_gen(asm_code, arm_load, p_des->reg_id, SP, p_addr->p_vreg->reg_id, 0, false);
        }
    }
    else {
        assert(p_addr->kind == reg);
        if (p_des->if_float)
            arm_vload_gen(asm_code, p_des->reg_id, p_addr->p_vreg->reg_id, 0, true);
        else
            arm_load_gen(asm_code, arm_load, p_des->reg_id, p_addr->p_vreg->reg_id, 0, 0, true);
    }
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
    return get_cond_type(p_vreg->p_instr_def->ir_binary.op);
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
        arm_cond_type type = get_jump_type(p_info, p_block->p_branch->p_exp->p_vreg);
        arm_block_jump_label_gen(p_info->asm_code, arm_b, type, p_func->name, p_block->p_branch->p_target_1->p_block->block_id);
        if (p_block->p_branch->p_target_2->p_block != p_next_block)
            arm_block_jump_label_gen(p_info->asm_code, arm_b, arm_al, p_func->name, p_block->p_branch->p_target_2->p_block->block_id);
        break;
    case ir_ret_branch:
        if (!p_block->p_branch->p_exp) {
            arm_out_func_gen(p_info, p_func->stack_size);
            break;
        }
        assert(p_block->p_branch->p_exp->kind == reg);
        size_t rd = 0;
        if (p_block->p_branch->p_exp->p_type->basic == type_f32)
            rd = R_NUM;
        if (p_block->p_branch->p_exp->kind == imme)
            mov_imme2reg(p_info, rd, p_block->p_branch->p_exp, false);
        else if (rd != p_block->p_branch->p_exp->p_vreg->reg_id)
            mov_reg2reg(p_info->asm_code, rd, p_block->p_branch->p_exp->p_vreg->reg_id, false);
        arm_out_func_gen(p_info, p_func->stack_size);
        break;
    case ir_abort_branch:
        break;
    }
}

static p_arm_codegen_info arm_codegen_info_gen(p_symbol_func p_func, char *asm_code) {
    p_arm_codegen_info p_info = malloc(sizeof(*p_info));
    p_info->asm_code = asm_code;
    p_info->p_func = p_func;
    p_info->mov_num = p_info->swap_num = 0;
    p_info->save_reg_r = malloc(sizeof(*p_info->save_reg_r) * R_NUM);
    p_info->save_reg_s = malloc(sizeof(*p_info->save_reg_s) * S_NUM);
    p_info->save_reg_r_num = p_func->save_reg_r_num;
    p_info->save_reg_s_num = p_func->save_reg_s_num;
    for (size_t i = 0; i < p_func->save_reg_r_num; i++)
        p_info->save_reg_r[i] = callee_save_reg_r[i];
    for (size_t i = 0; i < p_func->save_reg_s_num; i++)
        p_info->save_reg_s[i] = callee_save_reg_s[i];
    return p_info;
}
static void arm_codegen_info_drop(p_arm_codegen_info p_info) {
    free(p_info->save_reg_r);
    free(p_info->save_reg_s);
    free(p_info);
}

void arm_func_codegen(p_symbol_func p_func, char *asm_code) {
    if (list_head_alone(&p_func->block)) return;
    p_arm_codegen_info p_info = arm_codegen_info_gen(p_func, asm_code);

    arm_into_func_gen(p_info, p_func, p_func->stack_size);
    swap_in_func_param(p_info, p_func);
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_ir_basic_block p_next_block = list_entry(p_block_node->p_next, ir_basic_block, node);
        arm_basic_block_codegen(p_info, p_basic_block, p_next_block, p_func);
    }
    printf("%s: mov_num:%ld swap_num:%ld\n", p_func->name, p_info->mov_num, p_info->swap_num);
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
    list_for_each(p_node, &p_ir->constant) {
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
