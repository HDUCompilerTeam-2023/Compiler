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

#include <backend/arm/arm_struct_gen.h>
#include <ir_opt/lir_gen/arm_standard.h>

typedef struct arm_asm_gen_info arm_asm_gen_info, *p_arm_asm_gen_info;

struct arm_asm_gen_info {
    p_arm_block p_current_block;
    p_arm_func p_current_func;
    p_program p_program;
    size_t mov_num;
    size_t swap_num;
    size_t *save_reg_r;
    size_t *save_reg_s;
    size_t save_reg_r_num;
    size_t save_reg_s_num;
    p_symbol_func p_func;
};

static inline arm_reg vreg_arm_reg(p_ir_vreg p_vreg) {
    return p_vreg->reg_id;
}

static inline void mov_reg2reg(p_arm_asm_gen_info p_info, arm_reg rd, arm_reg rs) {
    p_arm_instr p_instr = NULL;
    if (!if_float_reg(rd) && !if_float_reg(rs))
        p_instr = arm_mov_instr_gen(arm_mov, arm_al, rd, arm_operand_reg_gen(rs));
    else
        p_instr = arm_vmov_instr_gen(rd, rs);
    arm_block_add_instr_tail(p_info->p_current_block, p_instr);
}

static void swap_reg(p_arm_asm_gen_info p_info, arm_reg *r1, arm_reg *r2, size_t num) {
    arm_reg *use_reg_count = malloc(sizeof(*use_reg_count) * REG_NUM);
    memset(use_reg_count, 0, REG_NUM * sizeof(*use_reg_count));
    arm_reg *r2_r1 = malloc(sizeof(*r2_r1) * REG_NUM);
    memset(r2_r1, -1, REG_NUM * sizeof(*r2_r1));
    bool *if_deal = malloc(sizeof(*if_deal) * REG_NUM);
    memset(if_deal, 0, sizeof(*if_deal) * REG_NUM);
    for (size_t i = 0; i < num; i++) {
        use_reg_count[r1[i]]++;
        r2_r1[r2[i]] = r1[i];
    }
    for (size_t i = 0; i < num; i++) {
        if (if_deal[r2[i]]) continue; // 在之前已经被处理过，但不可能在迭代的过程中发现已经被处理过，因为只有在被使用次数为零时才会处理
        arm_reg current_work_r1 = r1[i];
        arm_reg current_work_r2 = r2[i];
        while (use_reg_count[current_work_r2] == 0 && current_work_r1 != current_work_r2) {
            mov_reg2reg(p_info, current_work_r2, current_work_r1);
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
        mov_reg2reg(p_info, TMP, r2[i]);
        mov_reg2reg(p_info, r2[i], current_val_in[r1[i]]);
        mov_reg2reg(p_info, current_val_in[r1[i]], TMP);
        p_info->swap_num++;
        // else
        // arm_swap_gen(p_info->out_file, current_val_in[r1[i]], r2[i]);
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

static inline void mov_imm32_gen(p_arm_asm_gen_info p_info, arm_reg rd, I32CONST_t i32const) {
    p_arm_instr p_instr = arm_mov32_instr_gen(rd, NULL, i32const);
    arm_block_add_instr_tail(p_info->p_current_block, p_instr);
}

static void mov_int2reg(p_arm_asm_gen_info p_info, arm_reg rd, I32CONST_t i32const) {
    // 处理数字范围过大
    if (i32const > imm_16_max) {
        mov_imm32_gen(p_info, rd, i32const);
        return;
    }
    if (i32const < 0) // 负数
    {
        I32CONST_t neg = -i32const - 1;
        if (if_legal_rotate_imme12(neg)) {
            p_arm_instr p_instr = arm_mov_instr_gen(arm_mvn, arm_al, rd, arm_operand_imme_gen(neg));
            arm_block_add_instr_tail(p_info->p_current_block, p_instr);
            return;
        }
        mov_imm32_gen(p_info, rd, i32const);
        return;
    }
    p_arm_instr p_instr = arm_mov_instr_gen(arm_mov, arm_al, rd, arm_operand_imme_gen(i32const));
    arm_block_add_instr_tail(p_info->p_current_block, p_instr);
}

static void mov_float2reg(p_arm_asm_gen_info p_info, arm_reg rd, F32CONST_t floatconst) {
    // 浮点数放到标量寄存器
    assert(!if_float_reg(rd));
    mov_imm32_gen(p_info, rd, *(arm_imme *) (&floatconst));
}

static inline void mov_imme2reg(p_arm_asm_gen_info p_info, arm_reg rd, p_ir_operand p_operand) {
    assert(p_operand->kind == imme);
    if (p_operand->p_type->ref_level > 0) {
        assert(p_operand->p_vmem->is_global);
        p_arm_instr p_instr = arm_mov32_instr_gen(rd, arm_label_gen(p_operand->p_vmem->name), p_operand->offset);
        arm_block_add_instr_tail(p_info->p_current_block, p_instr);
        if (p_operand->offset >= 32768) {
            mov_int2reg(p_info, TMP, p_operand->offset);
            p_arm_instr p_add = arm_binary_instr_gen(arm_add, rd, rd, arm_operand_reg_gen(TMP));
            arm_block_add_instr_tail(p_info->p_current_block, p_add);
        }
        return;
    }
    switch (p_operand->p_type->basic) {
    case type_i32:
        mov_int2reg(p_info, rd, p_operand->i32const);
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
static inline void deal_sp_stack_size(p_arm_asm_gen_info p_info, arm_binary_op op, size_t stack_size) {
    if (!stack_size) return;
    if (!if_legal_rotate_imme12(stack_size)) {
        mov_int2reg(p_info, TMP, stack_size);
        p_arm_instr p_add = arm_binary_instr_gen(op, SP, SP, arm_operand_reg_gen(TMP));
        arm_block_add_instr_tail(p_info->p_current_block, p_add);
    }
    else {
        p_arm_instr p_add = arm_binary_instr_gen(op, SP, SP, arm_operand_imme_gen(stack_size));
        arm_block_add_instr_tail(p_info->p_current_block, p_add);
    }
}
static void arm_into_func_gen(p_arm_asm_gen_info p_info, p_symbol_func p_func, size_t stack_size) {
    p_info->p_current_block = p_info->p_current_func->p_into_func_block;
    p_arm_instr p_push_instr = arm_push_pop_instr_gen(arm_push);
    arm_push_pop_instr_init(&p_push_instr->push_pop_instr, p_info->save_reg_r, p_info->save_reg_r_num);
    arm_push_pop_instr_add(&p_push_instr->push_pop_instr, LR);
    arm_block_add_instr_tail(p_info->p_current_block, p_push_instr);
    p_arm_instr p_vpush_instr = arm_vpush_vpop_instr_gen(arm_vpush);
    arm_vpush_vpop_instr_init(&p_vpush_instr->vpush_vpop_instr, p_info->save_reg_s, p_info->save_reg_s_num);
    arm_block_add_instr_tail(p_info->p_current_block, p_vpush_instr);
    deal_sp_stack_size(p_info, arm_sub, stack_size);
}
static void arm_out_func_gen(p_arm_asm_gen_info p_info, size_t stack_size) {
    deal_sp_stack_size(p_info, arm_add, stack_size);
    p_arm_instr p_vpop = arm_vpush_vpop_instr_gen(arm_vpop);
    arm_vpush_vpop_instr_init(&p_vpop->vpush_vpop_instr, p_info->save_reg_s, p_info->save_reg_s_num);
    arm_block_add_instr_tail(p_info->p_current_block, p_vpop);
    p_arm_instr p_pop = arm_push_pop_instr_gen(arm_pop);
    arm_push_pop_instr_init(&p_pop->push_pop_instr, p_info->save_reg_r, p_info->save_reg_r_num);
    arm_push_pop_instr_add(&p_pop->push_pop_instr, PC);
    arm_block_add_instr_tail(p_info->p_current_block, p_pop);
}

void swap_in_call(p_arm_asm_gen_info p_info, p_ir_call_instr p_call_instr) {
    p_list_head p_node;
    size_t *src_reg = malloc(sizeof(*src_reg) * REG_NUM);
    size_t *des_reg = malloc(sizeof(*des_reg) * REG_NUM);
    p_ir_param *src_immes = malloc(sizeof(void *) * REG_NUM);
    size_t *des_imme_reg = malloc(sizeof(*des_imme_reg) * REG_NUM);

    size_t num = 0;
    size_t i = 0;
    size_t r = 0;
    size_t s = 0;
    list_for_each(p_node, &p_call_instr->param_list) {
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

        num++;
    }
    // TO DO: param in stack
    swap_reg(p_info, src_reg, des_reg, num);
    for (size_t j = 0; j < i; j++) {
        mov_imme2reg(p_info, des_imme_reg[j], src_immes[j]->p_param);
    }
    free(src_reg);
    free(des_reg);
    free(des_imme_reg);
    free(src_immes);
}

static void swap_in_phi(p_arm_asm_gen_info p_info, p_ir_basic_block p_basic_block, p_ir_basic_block_branch_target p_target) {
    arm_reg *src_reg = malloc(sizeof(*src_reg) * REG_NUM);
    arm_reg *des_reg = malloc(sizeof(*des_reg) * REG_NUM);
    size_t num = 0;
    p_list_head p_node1, p_node2;
    for (p_node1 = p_target->block_param.p_next, p_node2 = p_basic_block->basic_block_phis.p_next; p_node1 != &p_target->block_param
         && p_node2 != &p_basic_block->basic_block_phis;
         p_node1 = p_node1->p_next, p_node2 = p_node2->p_next) {
        p_ir_operand p_param = list_entry(p_node1, ir_bb_param, node)->p_bb_param;
        p_ir_vreg p_phi = list_entry(p_node2, ir_bb_phi, node)->p_bb_phi;
        assert(p_param);
        if (p_param->kind == imme) {
            continue;
        }
        // if (p_phi->p_bb_phi->kind == imme) continue;
        src_reg[num] = vreg_arm_reg(p_param->p_vreg);
        des_reg[num] = vreg_arm_reg(p_phi);
        num++;
    }
    swap_reg(p_info, src_reg, des_reg, num);
    for (p_node1 = &p_target->block_param, p_node2 = &p_basic_block->basic_block_phis; p_node1 != &p_target->block_param
         && p_node2 != &p_target->block_param;
         p_node1 = p_node1->p_next, p_node2 = p_node2->p_next) {
        // TODO param in mem
        p_ir_operand p_param = list_entry(p_node1, ir_bb_param, node)->p_bb_param;
        p_ir_vreg p_phi = list_entry(p_node2, ir_bb_phi, node)->p_bb_phi;
        if (p_param->kind == imme)
            mov_imme2reg(p_info, vreg_arm_reg(p_phi), p_param);
    }
    free(src_reg);
    free(des_reg);
}

static inline void swap_in_func_param(p_arm_asm_gen_info p_info, p_symbol_func p_func) {
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

static inline void arm_unary_instr_asm_gen(p_arm_asm_gen_info p_info, p_ir_unary_instr p_unary_instr) {
    p_ir_vreg p_des = p_unary_instr->p_des;
    arm_reg p_des_reg = vreg_arm_reg(p_des);
    p_ir_operand p_src = p_unary_instr->p_src;
    p_arm_instr p_new = NULL;
    switch (p_unary_instr->op) {
    case ir_minus_op:
        assert(p_unary_instr->p_src->kind == reg);
        arm_reg p_src_reg = vreg_arm_reg(p_src->p_vreg);
        if (p_des->if_float)
            p_new = arm_vneg_instr_gen(p_des_reg, p_src_reg);
        else
            p_new = arm_binary_instr_gen(arm_rsb, p_des_reg, p_src_reg, arm_operand_imme_gen(0));
        break;
    case ir_val_assign:
        if (p_unary_instr->p_src->kind == imme) {
            mov_imme2reg(p_info, p_des_reg, p_src);
            break;
        }
        mov_reg2reg(p_info, p_des_reg, vreg_arm_reg(p_src->p_vreg));
        break;
    case ir_i2f_op:
        p_new = arm_vcvt_instr_gen(arm_int2float, p_des_reg, vreg_arm_reg(p_src->p_vreg));
        break;
    case ir_f2i_op:
        p_new = arm_vcvt_instr_gen(arm_float2int, p_des_reg, vreg_arm_reg(p_src->p_vreg));
        break;
    case ir_ptr_add_sp:
        if (p_unary_instr->p_src->kind == imme) {
            p_new = arm_binary_instr_gen(arm_add, p_des_reg, SP, arm_operand_imme_gen(p_src->i32const));
            break;
        }
        p_new = arm_binary_instr_gen(arm_add, p_des_reg, SP, arm_operand_reg_gen(vreg_arm_reg(p_src->p_vreg)));
        break;
    }
    if (p_new)
        arm_block_add_instr_tail(p_info->p_current_block, p_new);
}
static void arm_binary_instr_asm_gen(p_arm_asm_gen_info p_info, p_ir_binary_instr p_binary_instr) {
    p_ir_vreg p_des = p_binary_instr->p_des;
    arm_reg p_des_reg = vreg_arm_reg(p_des);
    p_ir_operand p_src1 = p_binary_instr->p_src1;
    p_ir_operand p_src2 = p_binary_instr->p_src2;
    p_arm_instr p_new = NULL;
    switch (p_binary_instr->op) {
    case ir_add_op:
        if (p_src1->kind == imme) {
            assert(p_src2->kind == reg);
            assert(p_src1->p_type->ref_level == 0 && p_src1->p_type->basic == type_i32);
            p_new = arm_binary_instr_gen(arm_add, p_des_reg, vreg_arm_reg(p_src2->p_vreg), arm_operand_imme_gen(p_src1->i32const));
            break;
        }
        if (p_src2->kind == imme) {
            assert(p_src1->kind == reg);
            assert(p_src2->p_type->ref_level == 0 && p_src2->p_type->basic == type_i32);
            p_new = arm_binary_instr_gen(arm_add, p_des_reg, vreg_arm_reg(p_src1->p_vreg), arm_operand_imme_gen(p_src2->i32const));
            break;
        }
        if (p_des->if_float) {
            p_new = arm_vbinary_instr_gen(arm_vadd, p_des_reg, vreg_arm_reg(p_src1->p_vreg), vreg_arm_reg(p_src2->p_vreg));
            break;
        }
        p_new = arm_binary_instr_gen(arm_add, p_des_reg, vreg_arm_reg(p_src1->p_vreg), arm_operand_reg_gen(vreg_arm_reg(p_src2->p_vreg)));
        break;
    case ir_sub_op:
        if (p_src1->kind == imme) {
            assert(p_src2->kind == reg);
            assert(p_src1->p_type->ref_level == 0 && p_src1->p_type->basic == type_i32);
            p_new = arm_binary_instr_gen(arm_rsb, p_des_reg, vreg_arm_reg(p_src2->p_vreg), arm_operand_imme_gen(p_src1->i32const));
            break;
        }
        if (p_src2->kind == imme) {
            assert(p_src1->kind == reg);
            assert(p_src2->p_type->ref_level == 0 && p_src2->p_type->basic == type_i32);
            p_new = arm_binary_instr_gen(arm_sub, p_des_reg, vreg_arm_reg(p_src1->p_vreg), arm_operand_imme_gen(p_src2->i32const));
            break;
        }
        if (p_des->if_float) {
            p_new = arm_vbinary_instr_gen(arm_vsub, p_des_reg, vreg_arm_reg(p_src1->p_vreg), vreg_arm_reg(p_src2->p_vreg));
            break;
        }
        p_new = arm_binary_instr_gen(arm_sub, p_des_reg, vreg_arm_reg(p_src1->p_vreg), arm_operand_reg_gen(vreg_arm_reg(p_src2->p_vreg)));
        break;
    case ir_mul_op:
        if (p_des->if_float)
            p_new = arm_vbinary_instr_gen(arm_vmul, p_des_reg, vreg_arm_reg(p_src1->p_vreg), vreg_arm_reg(p_src2->p_vreg));
        else
            p_new = arm_mul_instr_gen(p_des_reg, vreg_arm_reg(p_src1->p_vreg), vreg_arm_reg(p_src2->p_vreg));
        break;
    case ir_div_op:
        if (p_des->if_float)
            p_new = arm_vbinary_instr_gen(arm_vdiv, p_des_reg, vreg_arm_reg(p_src1->p_vreg), vreg_arm_reg(p_src2->p_vreg));
        else
            p_new = arm_sdiv_instr_gen(p_des_reg, vreg_arm_reg(p_src1->p_vreg), vreg_arm_reg(p_src2->p_vreg));
        break;
    case ir_eq_op:
    case ir_neq_op:
    case ir_l_op:
    case ir_leq_op:
    case ir_g_op:
    case ir_geq_op:
        // can pointer compare ?
        assert(p_src1->kind == reg);
        if (p_src2->kind == imme) {
            assert(p_src2->p_type->basic == type_i32);
            if (p_src2->i32const < 0)
                p_new = arm_cmp_instr_gen(arm_cmn, vreg_arm_reg(p_src1->p_vreg), arm_operand_imme_gen(p_src2->i32const));
            else
                p_new = arm_cmp_instr_gen(arm_cmp, vreg_arm_reg(p_src1->p_vreg), arm_operand_imme_gen(p_src2->i32const));
        }
        else {
            if (p_src1->p_vreg->if_float)
                p_new = arm_vcmp_instr_gen(arm_vcmp, vreg_arm_reg(p_src1->p_vreg), vreg_arm_reg(p_src2->p_vreg));
            else
                p_new = arm_cmp_instr_gen(arm_cmp, vreg_arm_reg(p_src1->p_vreg), arm_operand_reg_gen(vreg_arm_reg(p_src2->p_vreg)));
        }
        if (!p_des->if_cond) {
            assert(p_new);
            arm_block_add_instr_tail(p_info->p_current_block, p_new);
            arm_cond_type type = get_cond_type(p_binary_instr->op);
            p_new = arm_mov_instr_gen(arm_mov, type, p_des_reg, arm_operand_imme_gen(1));
            arm_block_add_instr_tail(p_info->p_current_block, p_new);
            p_new = arm_mov_instr_gen(arm_mov, get_opposite_type(type), p_des_reg, arm_operand_imme_gen(0));
            arm_block_add_instr_tail(p_info->p_current_block, p_new);
            p_new = NULL;
        }
        break;
    case ir_mod_op:
        assert(0);
    }
    if (p_new)
        arm_block_add_instr_tail(p_info->p_current_block, p_new);
}

static inline p_list_head find_first_store(p_arm_asm_gen_info p_info, p_ir_instr p_instr, p_ir_call_instr p_call_instr) {
    if (list_head_alone(&p_call_instr->param_list))
        return &p_info->p_current_block->instr_list;
    p_ir_param p_param = list_entry(p_call_instr->param_list.p_prev, ir_param, node);
    if (!p_param->is_in_mem)
        return &p_info->p_current_block->instr_list;
    p_list_head p_node_asm = p_info->p_current_block->instr_list.p_prev;
    p_list_head p_node_ir = p_instr->node.p_prev;
    while (1) {
        p_ir_instr p_tmp = list_entry(p_node_ir, ir_instr, node);
        ir_instr_print(p_tmp);
        if (p_tmp->irkind == ir_store && p_tmp->ir_store.p_addr->kind == imme
            && p_tmp->ir_store.p_addr->i32const == -4)
            return p_node_asm;
        p_node_ir = p_node_ir->p_prev;
        p_node_asm = p_node_asm->p_prev;
    }
}
static inline size_t get_size(p_ir_call_instr p_call_instr) {
    if (list_head_alone(&p_call_instr->param_list))
        return 0;
    p_ir_param p_param = list_entry(p_call_instr->param_list.p_prev, ir_param, node);
    if (!p_param->is_in_mem)
        return 0;
    return -p_param->p_vmem->stack_offset;
}

static inline int cmp(const void *a, const void *b) {
    return *(int *) a - *(int *) b;
}
static inline void push_still_live(p_arm_asm_gen_info p_info, p_list_head p_insert_loc, p_ir_instr p_instr) {
    arm_reg *regs_r = malloc(sizeof(*regs_r) * REG_NUM);
    arm_reg *regs_s = malloc(sizeof(*regs_s) * REG_NUM);
    size_t push_num_r = 0;
    size_t push_num_s = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_instr->p_live_out->vreg_list) {
        p_ir_vreg p_live = list_entry(p_node, ir_vreg_list_node, node)->p_vreg;
        if (p_live == p_instr->ir_call.p_des)
            continue;
        if (!if_caller_save_reg(p_live->reg_id))
            continue;
        if (p_live->if_float)
            regs_s[push_num_s++] = vreg_arm_reg(p_live);
        else
            regs_r[push_num_r++] = vreg_arm_reg(p_live);
    }
    if ((push_num_r + push_num_s) & 1) {
        if (push_num_r)
            regs_r[push_num_r++] = TMP;
        else {
            arm_reg tmp = -1;
            if (p_instr->ir_call.p_des) // 不用目标做为tmp
                tmp = p_instr->ir_call.p_des->reg_id;
            size_t i;
            for (i = R_NUM; i < REG_NUM; i++) {
                if (i == tmp)
                    continue;
                size_t j;
                for (j = 0; j < push_num_s; j++)
                    if (i == regs_s[j])
                        break;
                if (j == push_num_s)
                    break;
            }
            regs_s[push_num_s++] = i;
        }
    }
    qsort(regs_r, push_num_r, sizeof(arm_reg), cmp);
    p_arm_instr p_push = arm_push_pop_instr_gen(arm_push);
    arm_push_pop_instr_init(&p_push->push_pop_instr, regs_r, push_num_r);
    list_add_prev(&p_push->node, p_insert_loc);
    qsort(regs_s, push_num_s, sizeof(arm_reg), cmp);

    size_t i = 0;
    while (i < push_num_s) {
        p_arm_instr p_vpush = arm_vpush_vpop_instr_gen(arm_vpush);
        list_add_prev(&p_vpush->node, p_insert_loc);
        arm_vpush_vpop_instr_add(&p_vpush->vpush_vpop_instr, regs_s[i]);
        i++;
        while (i < push_num_s && regs_s[i] == regs_s[i - 1] + 1) {
            arm_vpush_vpop_instr_add(&p_vpush->vpush_vpop_instr, regs_s[i]);
            i++;
        }
    }
    p_node = p_insert_loc;
    size_t new_sp = (push_num_r + push_num_s) * basic_type_get_size(type_i32);
    while (p_node != &p_info->p_current_block->instr_list) {
        p_arm_instr p_a_instr = list_entry(p_node, arm_instr, node);
        switch (p_a_instr->type) {
        case arm_mov_type:
            assert(p_a_instr->mov_instr.op == arm_mov);
            assert(p_a_instr->mov_instr.operand->if_imme);
            if (p_a_instr->mov_instr.rd == TMP) { // 偏移放到临时寄存器
                if (!(p_a_instr->mov_instr.operand->imme & (1 << ((sizeof(arm_imme) << 3) - 1))))
                    p_a_instr->mov_instr.operand->imme += new_sp;
            }
            break;
        case arm_mov32_type:
            if (p_a_instr->mov32_instr.label != NULL)
                break;
            if (p_a_instr->mov32_instr.rd != TMP)
                break;
            p_a_instr->mov32_instr.imme += new_sp;
            break;
        case arm_mem_type:
            if (p_a_instr->mem_instr.op == arm_load) {
                if (p_a_instr->mem_instr.base == SP) {
                    if (p_a_instr->mem_instr.offset->if_imme) {
                        assert(!(p_a_instr->mem_instr.offset->imme & (1 << ((sizeof(arm_imme) << 3) - 1))));
                        p_a_instr->mem_instr.offset->imme += new_sp;
                    }
                }
            }
            if (p_a_instr->mem_instr.op == arm_store) {
                if (p_a_instr->mem_instr.base == SP) {
                    if (p_a_instr->mem_instr.offset->if_imme)
                        if (!(p_a_instr->mem_instr.offset->imme & (1 << ((sizeof(arm_imme) << 3) - 1))))
                            p_a_instr->mem_instr.offset->imme += new_sp;
                }
            }
            break;
        case arm_vmem_type:
            if (p_a_instr->mem_instr.op == arm_vload) {
                if (p_a_instr->vmem_instr.base == SP) {
                    assert(!(p_a_instr->vmem_instr.offset & (1 << ((sizeof(arm_imme) << 3) - 1))));
                    p_a_instr->vmem_instr.offset += new_sp;
                }
                else {
                    assert(p_a_instr->vmem_instr.base == TMP);
                    assert(p_a_instr->vmem_instr.offset == 0);
                    p_a_instr->vmem_instr.offset += new_sp;
                }
            }
            if (p_a_instr->vmem_instr.op == arm_vstore) {
                if (p_a_instr->vmem_instr.base == SP) {
                    if (!(p_a_instr->vmem_instr.offset & ((1 << ((sizeof(arm_imme) << 3) - 1)))))
                        p_a_instr->vmem_instr.offset += new_sp;
                }
                else {
                    assert(p_a_instr->vmem_instr.base == TMP);
                    assert(p_a_instr->vmem_instr.offset == 0);
                    p_a_instr->vmem_instr.offset += new_sp;
                }
            }
            break;
        case arm_binary_type:
            assert(p_a_instr->binary_instr.op == arm_add);
            assert(p_a_instr->binary_instr.rs1 == SP);
            assert(p_a_instr->binary_instr.rd == TMP);
            break;
        case arm_vmov_type:
            break;
        default:
            assert(0);
        }
        p_node = p_node->p_next;
    }
    free(regs_r);
    free(regs_s);
}
static inline void pop_still_live(p_arm_asm_gen_info p_info, p_list_head p_push_loc) {
    p_list_head p_node = p_push_loc;
    while (p_node != &p_info->p_current_block->instr_list) {
        p_arm_instr p_a_instr = list_entry(p_node, arm_instr, node);
        p_node = p_node->p_prev;
        p_arm_instr p_pop, p_vpop;
        if (p_a_instr->type == arm_push_pop_type && p_a_instr->push_pop_instr.op == arm_push) {
            p_pop = arm_push_pop_instr_gen(arm_pop);
            arm_push_pop_instr_init(&p_pop->push_pop_instr, p_a_instr->push_pop_instr.regs, p_a_instr->push_pop_instr.num);
            arm_block_add_instr_tail(p_info->p_current_block, p_pop);
        }
        else if (p_a_instr->type == arm_vpush_vpop_type && p_a_instr->vpush_vpop_instr.op == arm_vpush) {
            p_vpop = arm_vpush_vpop_instr_gen(arm_vpop);
            arm_vpush_vpop_instr_init(&p_vpop->vpush_vpop_instr, p_a_instr->vpush_vpop_instr.regs, p_a_instr->vpush_vpop_instr.num);
            arm_block_add_instr_tail(p_info->p_current_block, p_vpop);
        }
        else
            break;
    }
}
static void arm_call_instr_asm_gen(p_arm_asm_gen_info p_info, p_ir_instr p_instr) {
    ir_instr_print(p_instr);
    p_ir_call_instr p_call_instr = &p_instr->ir_call;
    arm_reg rs = 0;
    if (p_call_instr->p_func->ret_type == type_f32)
        rs = R_NUM;
    p_list_head p_insert_loc = find_first_store(p_info, p_instr, p_call_instr);
    push_still_live(p_info, p_insert_loc, p_instr);
    p_list_head p_push_loc = p_insert_loc->p_prev;
    swap_in_call(p_info, p_call_instr);
    size_t size = get_size(p_call_instr);
    deal_sp_stack_size(p_info, arm_sub, size);
    p_arm_instr p_call = arm_call_instr_gen(p_call_instr->p_func->name);
    arm_block_add_instr_tail(p_info->p_current_block, p_call);
    if (p_call_instr->p_des) {
        if (rs != p_call_instr->p_des->reg_id)
            mov_reg2reg(p_info, vreg_arm_reg(p_call_instr->p_des), rs);
    }
    deal_sp_stack_size(p_info, arm_add, size);
    pop_still_live(p_info, p_push_loc);
}

static void arm_store_instr_gen(p_arm_asm_gen_info p_info, p_ir_store_instr p_store_instr) {
    assert(p_store_instr->p_src->kind == reg);
    p_ir_vreg p_src = p_store_instr->p_src->p_vreg;
    arm_reg p_src_reg = vreg_arm_reg(p_src);
    p_ir_operand p_addr = p_store_instr->p_addr;
    p_arm_instr p_new = NULL;
    if (p_store_instr->is_stack_ptr) {
        if (p_src->if_float) {
            if (p_addr->kind == imme)
                p_new = arm_vmem_instr_gen(arm_vstore, p_src_reg, SP, p_addr->i32const);
            else {
                p_arm_instr p_tmp = arm_binary_instr_gen(arm_add, TMP, SP, arm_operand_reg_gen(vreg_arm_reg(p_addr->p_vreg)));
                arm_block_add_instr_tail(p_info->p_current_block, p_tmp);
                p_new = arm_vmem_instr_gen(arm_vstore, p_src_reg, TMP, 0);
            }
        }
        else {
            if (p_addr->kind == imme)
                p_new = arm_mem_instr_gen(arm_store, p_src_reg, SP, arm_operand_imme_gen(p_addr->i32const));
            else
                p_new = arm_mem_instr_gen(arm_store, p_src_reg, SP, arm_operand_reg_gen(vreg_arm_reg(p_addr->p_vreg)));
        }
    }
    else {
        assert(p_addr->kind == reg);
        if (p_src->if_float)
            p_new = arm_vmem_instr_gen(arm_vstore, p_src_reg, vreg_arm_reg(p_addr->p_vreg), 0);
        else
            p_new = arm_mem_instr_gen(arm_store, p_src_reg, vreg_arm_reg(p_addr->p_vreg), arm_operand_imme_gen(0));
    }
    if (p_new)
        arm_block_add_instr_tail(p_info->p_current_block, p_new);
}

static void arm_load_instr_gen(p_arm_asm_gen_info p_info, p_ir_load_instr p_load_instr) {
    p_ir_vreg p_des = p_load_instr->p_des;
    arm_reg p_des_reg = vreg_arm_reg(p_des);
    p_ir_operand p_addr = p_load_instr->p_addr;
    p_arm_instr p_new = NULL;
    if (p_load_instr->is_stack_ptr) {
        if (p_des->if_float) {
            if (p_addr->kind == imme)
                p_new = arm_vmem_instr_gen(arm_vload, p_des_reg, SP, p_addr->i32const);
            else {
                p_arm_instr p_tmp = arm_binary_instr_gen(arm_add, TMP, SP, arm_operand_reg_gen(vreg_arm_reg(p_addr->p_vreg)));
                arm_block_add_instr_tail(p_info->p_current_block, p_tmp);
                p_new = arm_vmem_instr_gen(arm_vload, p_des_reg, TMP, 0);
            }
        }
        else {
            if (p_addr->kind == imme)
                p_new = arm_mem_instr_gen(arm_load, p_des_reg, SP, arm_operand_imme_gen(p_addr->i32const));
            else
                p_new = arm_mem_instr_gen(arm_load, p_des_reg, SP, arm_operand_reg_gen(vreg_arm_reg(p_addr->p_vreg)));
        }
    }
    else {
        assert(p_addr->kind == reg);
        if (p_des->if_float)
            p_new = arm_vmem_instr_gen(arm_vload, p_des_reg, vreg_arm_reg(p_addr->p_vreg), 0);
        else
            p_new = arm_mem_instr_gen(arm_load, p_des_reg, vreg_arm_reg(p_addr->p_vreg), arm_operand_imme_gen(0));
    }
    if (p_new)
        arm_block_add_instr_tail(p_info->p_current_block, p_new);
}
static void arm_instr_asm_gen(p_arm_asm_gen_info p_info, p_ir_instr p_instr) {
    switch (p_instr->irkind) {
    case ir_binary:
        arm_binary_instr_asm_gen(p_info, &p_instr->ir_binary);
        break;
    case ir_unary:
        arm_unary_instr_asm_gen(p_info, &p_instr->ir_unary);
        break;
    case ir_call:
        arm_call_instr_asm_gen(p_info, p_instr);
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

static arm_cond_type get_jump_type(p_arm_asm_gen_info p_info, p_ir_vreg p_vreg) {
    assert(p_vreg->if_cond);
    assert(p_vreg->def_type == instr_def);
    if (p_vreg->p_instr_def->irkind != ir_binary)
        return arm_ne;
    return get_cond_type(p_vreg->p_instr_def->ir_binary.op);
}

static void arm_basic_block_asm_gen(p_arm_asm_gen_info p_info, p_ir_basic_block p_block, p_ir_basic_block p_next_block, p_symbol_func p_func) {
    p_info->p_current_block = p_block->p_info;
    p_list_head p_node;
    list_for_each(p_node, &p_block->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        arm_instr_asm_gen(p_info, p_instr);
    }
    p_arm_instr p_jump;
    switch (p_block->p_branch->kind) {
    case ir_br_branch:
        swap_in_phi(p_info, p_block->p_branch->p_target_1->p_block, p_block->p_branch->p_target_1);
        p_jump = arm_jump_instr_gen(arm_b, arm_al, p_info->p_current_block, p_block->p_branch->p_target_1->p_block->p_info);
        arm_block_set_br1(p_info->p_current_block, p_jump);
        break;
    case ir_cond_branch:
        assert(p_block->p_branch->p_exp->kind == reg);
        arm_cond_type type = get_jump_type(p_info, p_block->p_branch->p_exp->p_vreg);
        p_jump = arm_jump_instr_gen(arm_b, type, p_info->p_current_block, p_block->p_branch->p_target_1->p_block->p_info);
        arm_block_set_br1(p_info->p_current_block, p_jump);
        p_jump = arm_jump_instr_gen(arm_b, arm_al, p_info->p_current_block, p_block->p_branch->p_target_2->p_block->p_info);
        arm_block_set_br2(p_info->p_current_block, p_jump);
        break;
    case ir_ret_branch:
        if (!p_block->p_branch->p_exp) {
            arm_out_func_gen(p_info, p_func->stack_size);
            break;
        }
        assert(p_block->p_branch->p_exp->kind == reg);
        assert(p_block->p_func->ret_type != type_void);
        arm_reg rd = 0;
        if (p_block->p_func->ret_type == type_f32)
            rd = R_NUM;
        p_ir_operand p_src = p_block->p_branch->p_exp;
        if (p_src->kind == imme)
            mov_imme2reg(p_info, rd, p_src);
        else if (rd != p_src->p_vreg->reg_id)
            mov_reg2reg(p_info, rd, vreg_arm_reg(p_src->p_vreg));
        arm_out_func_gen(p_info, p_func->stack_size);
        break;
    case ir_abort_branch:
        break;
    }
}

static p_arm_asm_gen_info arm_asm_gen_info_gen(p_program p_program, p_symbol_func p_func) {
    p_arm_asm_gen_info p_info = malloc(sizeof(*p_info));
    p_info->p_program = p_program;
    p_info->p_func = p_func;
    p_arm_func p_a_func = arm_func_gen(p_func->name);
    list_add_prev(&p_a_func->node, &p_info->p_program->arm_function);
    p_func->p_info = p_a_func;
    p_info->save_reg_r = malloc(sizeof(*p_info->save_reg_r) * R_NUM);
    p_info->save_reg_s = malloc(sizeof(*p_info->save_reg_s) * S_NUM);
    p_info->save_reg_r_num = p_func->save_reg_r_num;
    p_info->save_reg_s_num = p_func->save_reg_s_num;
    for (size_t i = 0; i < p_func->save_reg_r_num; i++)
        p_info->save_reg_r[i] = callee_save_reg_r[i];
    for (size_t i = 0; i < p_func->save_reg_s_num; i++)
        p_info->save_reg_s[i] = callee_save_reg_s[i];
    p_info->p_current_func = p_a_func;
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block, node);
        p_arm_block p_a_block = arm_block_gen(arm_block_label_gen(p_func->name, p_block->block_id));
        arm_func_add_block_tail(p_a_func, p_a_block);
        p_block->p_info = p_a_block;
    }
    p_info->mov_num = p_info->swap_num = 0;
    return p_info;
}
static void arm_asm_gen_info_drop(p_arm_asm_gen_info p_info) {
    free(p_info->save_reg_r);
    free(p_info->save_reg_s);
    free(p_info);
}

static inline void arm_func_asm_gen(p_program p_program, p_symbol_func p_func) {
    if (list_head_alone(&p_func->block)) return;
    p_arm_asm_gen_info p_info = arm_asm_gen_info_gen(p_program, p_func);

    arm_into_func_gen(p_info, p_func, p_func->stack_size);
    swap_in_func_param(p_info, p_func);
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_ir_basic_block p_next_block = list_entry(p_block_node->p_next, ir_basic_block, node);
        arm_basic_block_asm_gen(p_info, p_basic_block, p_next_block, p_func);
    }
    printf("%s: mov_num:%ld swap_num:%ld\n", p_func->name, p_info->mov_num, p_info->swap_num);
    arm_asm_gen_info_drop(p_info);
}

void ir_arm_asm_pass(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        arm_func_asm_gen(p_ir, p_func);
    }
}
