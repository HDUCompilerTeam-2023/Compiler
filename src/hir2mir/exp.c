#include <hir/exp.h>
#include <hir2mir.h>

#include <symbol_gen.h>
#include <program/gen.h>

static inline p_mir_operand hir2mir_sym_addr(p_hir2mir_info p_info, p_symbol_var p_var) {
    if (!p_var->p_vmem) {
        p_mir_vmem p_vmem = mir_vmem_sym_gen(p_var);
        if (p_var->is_global) {
            program_mir_vmem_add(p_info->p_program, p_vmem);
        }
        else {
            mir_func_vmem_add(p_info->p_func, p_vmem);
        }
        p_var->p_vmem = p_vmem;
    }

    return mir_operand_addr_gen(p_var->p_vmem);
}
static inline p_mir_operand hir2mir_exp_exec_gen(p_hir2mir_info p_info, p_hir_exp p_exp) {
    p_mir_operand p_src_1 = NULL;
    p_mir_operand p_src_2 = NULL;
    switch (p_exp->op) {
    case hir_exp_op_add:
    case hir_exp_op_sub:
    case hir_exp_op_mul:
    case hir_exp_op_div:
    case hir_exp_op_mod:
    case hir_exp_op_eq:
    case hir_exp_op_neq:
    case hir_exp_op_l:
    case hir_exp_op_leq:
    case hir_exp_op_g:
    case hir_exp_op_geq:
        assert(p_exp->p_src_1 && p_exp->p_src_2);
        p_src_1 = hir2mir_exp_gen(p_info, p_exp->p_src_1);
        p_src_2 = hir2mir_exp_gen(p_info, p_exp->p_src_2);
        break;
    case hir_exp_op_bool_not:
    case hir_exp_op_minus:
        assert(p_exp->p_src_1);
        p_src_1 = hir2mir_exp_gen(p_info, p_exp->p_src_1);
        break;
    case hir_exp_op_assign:
    case hir_exp_op_bool_and:
    case hir_exp_op_bool_or:
        assert(0);
    }

    p_mir_vreg p_vreg = mir_vreg_gen(p_exp->basic, 0);
    p_mir_instr p_instr = NULL;
    switch (p_exp->op) {
    case hir_exp_op_add:
        p_instr = mir_binary_instr_gen(mir_add_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_sub:
        p_instr = mir_binary_instr_gen(mir_sub_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_mul:
        p_instr = mir_binary_instr_gen(mir_mul_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_div:
        p_instr = mir_binary_instr_gen(mir_div_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_mod:
        p_instr = mir_binary_instr_gen(mir_mod_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_eq:
        p_instr = mir_binary_instr_gen(mir_eq_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_neq:
        p_instr = mir_binary_instr_gen(mir_neq_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_l:
        p_instr = mir_binary_instr_gen(mir_l_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_leq:
        p_instr = mir_binary_instr_gen(mir_leq_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_g:
        p_instr = mir_binary_instr_gen(mir_g_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_geq:
        p_instr = mir_binary_instr_gen(mir_geq_op, p_src_1, p_src_2, p_vreg);
        break;
    case hir_exp_op_bool_not:
        p_instr = mir_unary_instr_gen(mir_not_op, p_src_1, p_vreg);
        break;
    case hir_exp_op_minus:
        p_instr = mir_unary_instr_gen(mir_minus_op, p_src_1, p_vreg);
        break;
    case hir_exp_op_assign:
    case hir_exp_op_bool_and:
    case hir_exp_op_bool_or:
        assert(0);
    }
    hir2mir_info_add_instr(p_info, p_instr);
    return mir_operand_vreg_gen(p_vreg);
}

// 根据 p_exp 生成指令并返回最后一条指令的左值
p_mir_operand hir2mir_exp_gen(p_hir2mir_info p_info, p_hir_exp p_exp) {
    if (!p_exp) return NULL;
    switch (p_exp->kind) {
    case hir_exp_num: // 若是常量 直接返回该常量对应的操作数
        if (p_exp->basic == type_int) {
            return mir_operand_int_gen(p_exp->intconst);
        }
        if (p_exp->basic == type_float) {
            return mir_operand_float_gen(p_exp->floatconst);
        }
        if (p_exp->basic == type_str) {
            return mir_operand_str_gen(p_exp->p_str);
        }
        assert(0);
    case hir_exp_val: {
        // 若是变量 直接返回该变量对应的操作数
        p_mir_operand p_addr_operand = hir2mir_sym_addr(p_info, p_exp->p_var);
        if ((list_head_alone(&p_exp->p_var->p_type->array) && p_exp->p_var->p_type->ref_level == 0)|| p_exp->p_var->p_type->ref_level > 0) {
            p_mir_vreg p_val = mir_vreg_gen(p_addr_operand->b_type, p_addr_operand->ref_level - 1);
            p_mir_operand p_val_operand = mir_operand_vreg_gen(p_val);
            p_mir_instr p_load_val = mir_load_instr_gen(p_addr_operand, NULL, p_val);
            hir2mir_info_add_instr(p_info, p_load_val);
            if ((list_head_alone(&p_exp->p_var->p_type->array) && p_exp->p_var->p_type->ref_level == 0)) return p_val_operand;
            p_addr_operand = p_val_operand;
        }
        p_mir_operand p_offset = hir2mir_exp_gen(p_info, p_exp->p_offset);
        if (!list_head_alone(&p_exp->p_type->array) || p_exp->p_type->ref_level > 0) {
            if (!p_offset) return p_addr_operand;
            p_mir_vreg p_des = mir_vreg_gen(p_addr_operand->b_type, p_addr_operand->ref_level);
            p_mir_instr p_instr = mir_binary_instr_gen(mir_add_op, p_addr_operand, p_offset, p_des);
            hir2mir_info_add_instr(p_info, p_instr);
            return mir_operand_vreg_gen(p_des);
        }
        p_mir_vreg p_des = mir_vreg_gen(p_addr_operand->b_type, p_addr_operand->ref_level - 1);
        p_mir_instr p_load = mir_load_instr_gen(p_addr_operand, p_offset, p_des);
        hir2mir_info_add_instr(p_info, p_load);
        return mir_operand_vreg_gen(p_des);
    }
    case hir_exp_exec: {
        if (p_exp->op == hir_exp_op_assign) {
            assert(p_exp->p_src_1 && p_exp->p_src_2);
            assert(p_exp->p_src_1->kind == hir_exp_val);
            p_mir_operand p_src = hir2mir_exp_gen(p_info, p_exp->p_src_2);
            p_mir_operand p_addr = hir2mir_sym_addr(p_info, p_exp->p_src_1->p_var);
            p_mir_operand p_off = hir2mir_exp_gen(p_info, p_exp->p_src_1->p_offset);
            if (p_off && p_exp->p_src_1->p_var->p_type->ref_level > 0) {
                p_mir_vreg p_val = mir_vreg_gen(p_addr->b_type, p_addr->ref_level - 1);
                p_mir_operand p_val_operand = mir_operand_vreg_gen(p_val);
                p_mir_instr p_load_val = mir_load_instr_gen(p_addr, NULL, p_val);
                hir2mir_info_add_instr(p_info, p_load_val);
                p_addr = p_val_operand;
            }
            p_mir_instr p_instr = mir_store_instr_gen(p_addr, p_off, p_src);
            hir2mir_info_add_instr(p_info, p_instr);
            return NULL;
        }
        return hir2mir_exp_exec_gen(p_info, p_exp);
    }
    case hir_exp_call: {
        p_mir_vreg p_des = mir_vreg_gen(p_exp->p_func->ret_type, 0);

        p_mir_param_list p_m_param_list = hir2mir_param_list_gen(p_info, p_exp->p_param_list);

        p_mir_instr p_new_instr = mir_call_instr_gen(p_exp->p_func, p_m_param_list, p_des);
        hir2mir_info_add_instr(p_info, p_new_instr);

        return mir_operand_vreg_gen(p_des);
    }
    }
}
// exp 正确则跳向 true, 错误跳向 false
p_mir_operand hir2mir_exp_cond_gen(p_hir2mir_info p_info, p_mir_basic_block p_true_block, p_mir_basic_block p_false_block, p_hir_exp p_exp) {
    assert(p_exp);
    if (p_exp->op == hir_exp_op_bool_or) {
        p_mir_basic_block p_new_false_block = mir_basic_block_gen();
        // 在当前 block 生成 左边代码
        hir2mir_exp_cond_gen(p_info, p_true_block, p_new_false_block, p_exp->p_src_1);

        // 在新block 生成右边代码， 该block 也是左边的 false block
        hir2mir_info_add_basic_block(p_info, p_new_false_block);
        hir2mir_exp_cond_gen(p_info, p_true_block, p_false_block, p_exp->p_src_2);
    }
    else if (p_exp->op == hir_exp_op_bool_and) {
        p_mir_basic_block p_new_true_block = mir_basic_block_gen();
        // 在当前 block 生成 左边代码
        hir2mir_exp_cond_gen(p_info, p_new_true_block, p_false_block, p_exp->p_src_1);
        // 在新block 生成右边代码， 该block 也是左边的 true block
        hir2mir_info_add_basic_block(p_info, p_new_true_block);
        hir2mir_exp_cond_gen(p_info, p_true_block, p_false_block, p_exp->p_src_2);
    }
    else {
        p_mir_operand p_cond = hir2mir_exp_gen(p_info, p_exp);
        mir_basic_block_set_cond(p_info->p_current_basic_block, p_cond, p_true_block, p_false_block);
    }
    return NULL;
}
