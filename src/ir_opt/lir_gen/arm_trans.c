#include <ir_opt/lir_gen/arm_trans.h>
#include <ir_opt/lir_gen/arm_standard.h>
#include <ir_gen.h>
#include <program/def.h>
#include <symbol_gen.h>

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

static inline bool if_need_float(p_ir_vreg p_vreg) {
    return (p_vreg->p_type->basic == type_f32 && p_vreg->p_type->ref_level == 0);
}
static inline void set_float_reg(p_ir_vreg p_vreg) {
    p_vreg->if_float = true;
}

// 默认已经经过常量传播
static void deal_binary_instr(p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_binary_instr p_binary_instr = &p_instr->ir_binary;
    switch (p_binary_instr->op) {
    case ir_add_op:
    case ir_sub_op:
        check_imme2reg(p_binary_instr->p_src1, p_instr, p_func);
        check_imme2reg(p_binary_instr->p_src2, p_instr, p_func);
        if (if_need_float(p_binary_instr->p_des)) {
            assert(p_binary_instr->p_src1->kind == reg);
            assert(p_binary_instr->p_src2->kind == reg);
            set_float_reg(p_binary_instr->p_src1->p_vreg);
            set_float_reg(p_binary_instr->p_src2->p_vreg);
            set_float_reg(p_binary_instr->p_des);
        }
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
        assert(p_binary_instr->p_src1->kind == reg);
        if (if_need_float(p_binary_instr->p_src1->p_vreg)) {
            assert(p_binary_instr->p_src2->kind == reg);
            set_float_reg(p_binary_instr->p_src1->p_vreg);
            set_float_reg(p_binary_instr->p_src2->p_vreg);
        }
        break;
    case ir_mul_op:
    case ir_div_op:
        assert(p_binary_instr->p_src1->p_type->ref_level == 0);
        assert(p_binary_instr->p_src2->p_type->ref_level == 0);
        imme2reg(p_binary_instr->p_src1, p_instr, p_func);
        imme2reg(p_binary_instr->p_src2, p_instr, p_func);
        if (p_binary_instr->p_des->p_type->basic == type_f32) {
            set_float_reg(p_binary_instr->p_src1->p_vreg);
            set_float_reg(p_binary_instr->p_src2->p_vreg);
            set_float_reg(p_binary_instr->p_des);
        }
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
        assert(p_unary_instr->p_src->kind == reg);
        assert(p_unary_instr->p_src->p_type->ref_level == 0);
        if (p_unary_instr->p_src->p_type->basic == type_f32) {
            set_float_reg(p_unary_instr->p_des);
            set_float_reg(p_unary_instr->p_src->p_vreg);
        }
        break;
    case ir_val_assign:
        if (p_unary_instr->p_src->kind == imme && p_unary_instr->p_src->p_type->ref_level == 0 && p_unary_instr->p_src->p_type->basic == type_f32)
            if (p_unary_instr->p_des->if_float) {
                p_ir_vreg p_new_des = ir_vreg_copy(p_unary_instr->p_des);
                p_new_des->if_float = false;
                symbol_func_vreg_add(p_func, p_new_des);
                p_ir_instr p_new_assign = ir_unary_instr_gen(ir_val_assign, p_unary_instr->p_src, p_new_des);
                list_add_prev(&p_new_assign->node, &p_instr->node);
                p_unary_instr->p_src = ir_operand_vreg_gen(p_new_des);
            }
        break;
    case ir_i2f_op:
        assert(p_unary_instr->p_src->kind == reg);
        assert(p_unary_instr->p_src->p_type->ref_level == 0);
        if(p_unary_instr->p_src->p_vreg->if_float){
            set_float_reg(p_unary_instr->p_des);
            break;
        }
        // f32 = i32 -> i32(s) = i32; f32(s) = i32(s)
        p_ir_vreg p_new_src = ir_vreg_gen(symbol_type_var_gen(type_i32));
        symbol_func_vreg_add(p_func, p_new_src);
        p_ir_instr p_assign = ir_unary_instr_gen(ir_val_assign, p_unary_instr->p_src, p_new_src);
        list_add_prev(&p_assign->node, &p_instr->node);
        p_unary_instr->p_src = ir_operand_vreg_gen(p_new_src);
        set_float_reg(p_new_src);
        set_float_reg(p_unary_instr->p_des);
        break;
    case ir_f2i_op:
        assert(p_unary_instr->p_src->kind == reg);
        assert(p_unary_instr->p_src->p_type->ref_level == 0);
        if(p_unary_instr->p_des->if_float){
            set_float_reg(p_unary_instr->p_src->p_vreg);
            break;
        }
        // i32 = f32 -> i32(s) = f32(s); i32 = i32(s)
        p_ir_vreg p_new_des = ir_vreg_gen(symbol_type_var_gen(type_i32));
        symbol_func_vreg_add(p_func, p_new_des);
        p_ir_instr p_new_float2int = ir_unary_instr_gen(ir_f2i_op, p_unary_instr->p_src, p_new_des);
        list_add_prev(&p_new_float2int->node, &p_instr->node);
        p_unary_instr->op = ir_val_assign;
        p_unary_instr->p_src = ir_operand_vreg_gen(p_new_des);
        set_float_reg(p_new_des);
        set_float_reg(p_new_float2int->ir_unary.p_src->p_vreg);
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
    p_list_head p_node;
    switch (p_instr->irkind) {
    case ir_binary:
        deal_binary_instr(p_instr, p_func);
        break;
    case ir_unary:
        deal_unary_instr(p_instr, p_func);
        break;
    case ir_call:
        // 返回值在代码生成的时候新增 mov, 不一定放浮点寄存器
        // 调用时浮点数放浮点寄存器
        list_for_each(p_node, &p_instr->ir_call.p_param_list->param) {
            p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
            check_imme2reg(p_param, p_instr, p_func);
            if (p_param->kind == reg && if_need_float(p_param->p_vreg))
                set_float_reg(p_param->p_vreg);
        }
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
    list_for_each_tail(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        switch (p_basic_block->p_branch->kind) {
        case ir_br_branch:
            break;
        case ir_cond_branch:
            imme2reg(p_basic_block->p_branch->p_exp, list_entry(&p_basic_block->instr_list, ir_instr, node), p_func);
            break;
        case ir_ret_branch:
            if (p_basic_block->p_branch->p_exp){
                imme2reg(p_basic_block->p_branch->p_exp, list_entry(&p_basic_block->instr_list, ir_instr, node), p_func);
            }
            break;
        case ir_abort_branch:
            break;
        }
        p_list_head p_instr_node;
        list_for_each_tail(p_instr_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            deal_instr(p_instr, p_func);
        }
    }
    symbol_func_set_block_id(p_func);
}

void arm_lir_trans_pass(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        arm_lir_func_trans(p_func);
    }
}