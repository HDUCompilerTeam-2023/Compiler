#include <ir_opt/lir_gen/arm_trans.h>

#include <ir_gen.h>
#include <program/def.h>
#include <symbol_gen.h>

// value循环右移bits位
#define ror(value, bits) ((value >> bits) | (value << (sizeof(value) * 8 - bits)))

static const I32CONST_t imm_8_max = 255;

// 是否由八位循环右移偶数位得到
static inline bool if_legal_rotate_imme12(I32CONST_t i32const) {
    if (i32const == 0) return true;
    if (i32const < 0) // 负数
        i32const = -i32const;
    uint32_t trans = i32const;
    uint32_t window = ~imm_8_max;
    for (size_t i = 0; i < 16; i++) {
        if (!(window & trans))
            return true;
        window = ror(window, 2);
    }
    return false;
}

static inline void imme2reg(p_ir_operand p_operand, p_ir_instr p_instr, p_symbol_func p_func) {
    if (p_operand->kind != imme) return;

    p_ir_vreg p_new_src = ir_vreg_gen(symbol_type_copy(p_operand->p_type));
    p_ir_instr p_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_copy(p_operand), p_new_src);
    symbol_func_vreg_add(p_func, p_new_src);
    list_add_prev(&p_assign->node, &p_instr->node);
    p_operand->kind = reg;
    p_operand->p_vreg = p_new_src;
}

static inline void check_imme2reg(p_ir_operand p_operand, p_ir_instr p_instr, p_symbol_func p_func) {
    if (p_operand->kind != imme) return;

    if (p_operand->p_type->ref_level > 0) {
        if (p_operand->p_vmem->is_global)
            imme2reg(p_operand, p_instr, p_func);
        return;
    }
    switch (p_operand->p_type->basic) {
    case type_i32:
        if (!if_legal_rotate_imme12(p_operand->i32const))
            imme2reg(p_operand, p_instr, p_func);
        break;
    case type_f32:
        imme2reg(p_operand, p_instr, p_func);
        break;
    case type_str:
        imme2reg(p_operand, p_instr, p_func);
        break;
    case type_void:
        assert(0);
    }
}

// 默认已经经过常量传播
static void deal_binary_instr(p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_binary_instr p_binary_instr = &p_instr->ir_binary;
    switch (p_binary_instr->op) {
    case ir_add_op:
    case ir_sub_op:
        check_imme2reg(p_binary_instr->p_src1, p_instr, p_func);
        check_imme2reg(p_binary_instr->p_src2, p_instr, p_func);
        break;
    case ir_and_op:
    case ir_or_op:
    case ir_eq_op:
    case ir_neq_op:
    case ir_l_op:
    case ir_leq_op:
    case ir_g_op:
    case ir_geq_op:
        check_imme2reg(p_binary_instr->p_src1, p_instr, p_func);
        check_imme2reg(p_binary_instr->p_src2, p_instr, p_func);
        if (p_binary_instr->p_src1->kind == imme) {
            if (p_binary_instr->p_src2->kind == imme) { // 如果指针不能比较 在常量折叠后不应出现这个，之后删
                imme2reg(p_binary_instr->p_src1, p_instr, p_func);
                break;
            }
            p_ir_operand temp = p_binary_instr->p_src1;
            p_binary_instr->p_src1 = p_binary_instr->p_src2;
            p_binary_instr->p_src2 = temp;
            switch (p_binary_instr->op) {
            case ir_eq_op:
                p_binary_instr->op = ir_neq_op;
                break;
            case ir_neq_op:
                p_binary_instr->op = ir_eq_op;
                break;
            case ir_l_op:
                p_binary_instr->op = ir_geq_op;
                break;
            case ir_leq_op:
                p_binary_instr->op = ir_g_op;
                break;
            case ir_g_op:
                p_binary_instr->op = ir_leq_op;
                break;
            case ir_geq_op:
                p_binary_instr->op = ir_l_op;
                break;
            default:
                break;
            }
        }
        break;
    case ir_mul_op:
    case ir_div_op:
        assert(p_binary_instr->p_src1->p_type->ref_level == 0);
        assert(p_binary_instr->p_src2->p_type->ref_level == 0);
        imme2reg(p_binary_instr->p_src1, p_instr, p_func);
        imme2reg(p_binary_instr->p_src2, p_instr, p_func);
        break;
    case ir_mod_op:
        assert(0);
        break;
    }
}
static void deal_unary_instr(p_ir_instr p_instr, p_symbol_func p_func) {
    assert(p_instr->irkind == ir_unary);
    p_ir_unary_instr p_unary_instr = &p_instr->ir_unary;
    switch (p_unary_instr->op) {
    case ir_minus_op:
    case ir_val_assign:
        break;
    case ir_i2f_op:
    case ir_f2i_op:
        break;
    }
}

static void deal_load_instr(p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_load_instr p_load_instr = &p_instr->ir_load;
    check_imme2reg(p_load_instr->p_addr, p_instr, p_func);
    assert(!p_load_instr->p_offset);
}

static void deal_store_instr(p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_store_instr p_store_instr = &p_instr->ir_store;
    imme2reg(p_store_instr->p_src, p_instr, p_func); // store 的源必须为寄存器
    check_imme2reg(p_store_instr->p_addr, p_instr, p_func);
    assert(!p_store_instr->p_offset);
}

static void deal_instr(p_ir_instr p_instr, p_symbol_func p_func) {
    switch (p_instr->irkind) {
    case ir_binary:
        deal_binary_instr(p_instr, p_func);
        break;
    case ir_unary:
        deal_unary_instr(p_instr, p_func);
        break;
    case ir_call:
        break;
    case ir_load:
        deal_load_instr(p_instr, p_func);
        break;
    case ir_store:
        deal_store_instr(p_instr, p_func);
        break;
    case ir_gep:
        assert(0);
        break;
    }
}

void arm_lir_func_trans(p_symbol_func p_func) {
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            deal_instr(p_instr, p_func);
        }
        switch (p_basic_block->p_branch->kind) {
        case ir_br_branch:
            break;
        case ir_cond_branch:
            imme2reg(p_basic_block->p_branch->p_exp, list_entry(&p_basic_block->instr_list, ir_instr, node), p_func);
            break;
        case ir_ret_branch:
            if (p_basic_block->p_branch->p_exp)
                imme2reg(p_basic_block->p_branch->p_exp, list_entry(&p_basic_block->instr_list, ir_instr, node), p_func);
            break;
        case ir_abort_branch:
            break;
        }
    }
    symbol_func_set_vreg_id(p_func);
}

void arm_lir_trans_pass(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        arm_lir_func_trans(p_func);
    }
}