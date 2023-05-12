#include "symbol.h"
#include <hir/exp.h>
#include <hir/func.h>
#include <hir2mir.h>

#include <symbol/sym.h>
#include <symbol/type.h>

static inline p_mir_vreg hir2mir_sym_addr(p_hir2mir_info p_info, p_symbol_sym p_sym) {
    size_t id = p_sym->id;
    p_mir_vreg *table;
    if (p_sym->is_global) {
        table = p_info->global_addr_table;
    }
    else {
        table = p_info->local_addr_table;
    }
    p_mir_vreg p_addr = table[id];

    if (!p_addr) {
        p_mir_vmem p_vmem = NULL;
        if (p_sym->is_global) {
            p_vmem = p_info->p_program_info->global_vmem_table[id];
            if (!p_vmem) {
                p_vmem = p_info->p_program_info->global_vmem_table[id] = mir_vmem_sym_gen(p_sym);
                mir_program_vmem_add(p_info->p_program_info->p_program, p_vmem);
            }
        }
        else {
            p_vmem = mir_vmem_sym_gen(p_sym);
            mir_func_vmem_add(p_info->p_func, p_vmem);
        }
        p_addr = mir_vreg_gen(p_vmem->b_type, p_vmem->ref_level + 1);
        hir2mir_info_add_instr(p_info, mir_addr_instr_gen(p_vmem, p_addr));
        while (p_addr->ref_level > 1) {
            p_mir_operand p_operand = mir_operand_vreg_gen(p_addr);
            p_addr = mir_vreg_gen(p_addr->b_type, p_addr->ref_level - 1);
            hir2mir_info_add_instr(p_info, mir_load_instr_gen(p_operand, NULL, p_addr));
        }

        table[id] = p_addr;
    }

    return p_addr;
}

// 根据 p_exp 生成指令并返回最后一条指令的左值
p_mir_operand hir2mir_exp_get_operand(p_hir2mir_info p_info, p_hir_exp p_exp) {
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
        p_mir_vreg p_addr_vreg = hir2mir_sym_addr(p_info, p_exp->p_sym);
        p_mir_operand p_addr_operand = mir_operand_vreg_gen(p_addr_vreg);
        p_mir_operand p_offset = NULL;
        if (p_exp->p_offset) // 若是数组元素赋值 需要新增一条语句将数组元素赋值给临时变量
        {
            p_offset = hir2mir_exp_get_operand(p_info, p_exp->p_offset);
            if (p_exp->p_type->kind == type_arrary) {
                p_mir_vreg p_des = mir_vreg_gen(p_addr_vreg->b_type, p_addr_vreg->ref_level);
                p_mir_instr p_instr = mir_binary_instr_gen(mir_add_op, p_addr_operand, p_offset, p_des);
                hir2mir_info_add_instr(p_info, p_instr);
                return mir_operand_vreg_gen(p_des);
            }
        }
        p_mir_vreg p_des = mir_vreg_gen(p_addr_vreg->b_type, p_addr_vreg->ref_level - 1);
        p_mir_instr p_load = mir_load_instr_gen(p_addr_operand, p_offset, p_des);
        hir2mir_info_add_instr(p_info, p_load);
        return mir_operand_vreg_gen(p_des);
    }
    default:
        return mir_operand_vreg_gen(mir_instr_get_des(hir2mir_exp_gen(p_info, p_exp)));
    }
}

p_mir_instr hir2mir_exp_gen(p_hir2mir_info p_info, p_hir_exp p_exp) {
    if (!p_exp) return NULL;
    switch (p_exp->kind) {
    case hir_exp_exec:
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
            return hir2mir_exp_exec_gen(p_info, p_exp);
        case hir_exp_op_assign:
            return hir2mir_exp_assign_gen(p_info, p_exp);
        case hir_exp_op_bool_not:
        case hir_exp_op_minus:
            return hir2mir_exp_uexec_gen(p_info, p_exp);
        default: // 逻辑运算先进行 condbr判断, 不会到达此
            assert(0);
        }
    case hir_exp_call:
        return hir2mir_exp_call_gen(p_info, p_exp);
    case hir_exp_num:
    case hir_exp_val:
        return NULL;
    }
}

p_mir_instr hir2mir_exp_exec_gen(p_hir2mir_info p_info, p_hir_exp p_exp) {
    assert(p_exp && p_exp->p_src_1 && p_exp->p_src_2);
    p_mir_operand p_operand1 = hir2mir_exp_get_operand(p_info, p_exp->p_src_1);
    p_mir_operand p_operand2 = hir2mir_exp_get_operand(p_info, p_exp->p_src_2);

    basic_type b_type;
    switch (p_exp->op) {
    case hir_exp_op_add:
    case hir_exp_op_sub:
    case hir_exp_op_mul:
    case hir_exp_op_div:
    case hir_exp_op_mod:
        b_type = p_exp->basic;
        break;
    case hir_exp_op_eq:
    case hir_exp_op_neq:
    case hir_exp_op_g:
    case hir_exp_op_geq:
    case hir_exp_op_l:
    case hir_exp_op_leq:
        b_type = type_int;
        break;
    default:
        assert(0);
    }
    p_mir_vreg p_vreg = mir_vreg_gen(b_type, 0);

    mir_instr_type mir_type;
    switch (p_exp->op) {
    case hir_exp_op_add:
        mir_type = mir_add_op;
        break;
    case hir_exp_op_sub:
        mir_type = mir_sub_op;
        break;
    case hir_exp_op_mul:
        mir_type = mir_mul_op;
        break;
    case hir_exp_op_div:
        mir_type = mir_div_op;
        break;
    case hir_exp_op_mod:
        mir_type = mir_mod_op;
        break;
    case hir_exp_op_eq:
        mir_type = mir_eq_op;
        break;
    case hir_exp_op_neq:
        mir_type = mir_neq_op;
        break;
    case hir_exp_op_g:
        mir_type = mir_g_op;
        break;
    case hir_exp_op_geq:
        mir_type = mir_geq_op;
        break;
    case hir_exp_op_l:
        mir_type = mir_l_op;
        break;
    case hir_exp_op_leq:
        mir_type = mir_leq_op;
        break;
    default:
        assert(0);
    }
    p_mir_instr p_new_instr = mir_binary_instr_gen(mir_type, p_operand1, p_operand2, p_vreg);
    hir2mir_info_add_instr(p_info, p_new_instr);
    return p_new_instr;
}

p_mir_instr hir2mir_exp_uexec_gen(p_hir2mir_info p_info, p_hir_exp p_exp) {
    assert(p_exp && p_exp->p_src_1);
    p_mir_operand p_operand = hir2mir_exp_get_operand(p_info, p_exp->p_src_1);
    p_mir_vreg p_des = NULL;
    p_mir_instr p_instr = NULL;
    switch (p_exp->op) {
    case hir_exp_op_bool_not:
        // 需要转换为 int 型
        p_des = mir_vreg_gen(type_int, 0);
        p_instr = mir_unary_instr_gen(mir_not_op, p_operand, p_des);
        break;
    case hir_exp_op_minus:
        p_des = mir_vreg_gen(mir_operand_get_basic_type(p_operand), 0);
        p_instr = mir_unary_instr_gen(mir_minus_op, p_operand, p_des);
        break;
    default:
        assert(0);
    }
    hir2mir_info_add_instr(p_info, p_instr);
    return p_instr;
}
// exp 正确则跳向 true, 错误跳向 false
p_mir_instr hir2mir_exp_cond_gen(p_hir2mir_info p_info, p_mir_basic_block p_true_block, p_mir_basic_block p_false_block, p_hir_exp p_exp) {
    assert(p_exp);
    p_mir_instr p_new_instr = NULL;
    if (p_exp->op == hir_exp_op_bool_or) {
        p_mir_basic_block p_new_false_block = mir_basic_block_gen();
        // 在当前 block 生成 左边代码
        hir2mir_exp_cond_gen(p_info, p_true_block, p_new_false_block, p_exp->p_src_1);

        // 在新block 生成右边代码， 该block 也是左边的 false block
        hir2mir_info_add_basic_block(p_info, p_new_false_block);
        p_new_instr = hir2mir_exp_cond_gen(p_info, p_true_block, p_false_block, p_exp->p_src_2);
    }
    else if (p_exp->op == hir_exp_op_bool_and) {
        p_mir_basic_block p_new_true_block = mir_basic_block_gen();
        // 在当前 block 生成 左边代码
        hir2mir_exp_cond_gen(p_info, p_new_true_block, p_false_block, p_exp->p_src_1);
        // 在新block 生成右边代码， 该block 也是左边的 true block
        hir2mir_info_add_basic_block(p_info, p_new_true_block);
        p_new_instr = hir2mir_exp_cond_gen(p_info, p_true_block, p_false_block, p_exp->p_src_2);
    }
    else {
        p_mir_operand p_cond = hir2mir_exp_get_operand(p_info, p_exp);
        p_new_instr = mir_condbr_instr_gen(p_info->p_current_basic_block, p_cond, p_true_block, p_false_block);
        hir2mir_info_add_instr(p_info, p_new_instr);
    }
    return p_new_instr;
}

p_mir_instr hir2mir_exp_assign_gen(p_hir2mir_info p_info, p_hir_exp p_exp) {
    assert(p_exp);
    assert(p_exp->p_src_1 && p_exp->p_src_2);
    assert(p_exp->p_src_1->kind == hir_exp_val);

    p_mir_instr p_new_instr = NULL;
    p_mir_operand p_src = hir2mir_exp_get_operand(p_info, p_exp->p_src_2);
    p_mir_operand p_addr = mir_operand_vreg_gen(hir2mir_sym_addr(p_info, p_exp->p_src_1->p_sym));
    p_mir_operand p_offset = NULL;
    if (p_exp->p_src_1->p_offset) { // 左值为数组对应指令为 数组赋值指令
        p_offset = hir2mir_exp_get_operand(p_info, p_exp->p_src_1->p_offset);
    }
    p_new_instr = mir_store_instr_gen(p_addr, p_offset, p_src);
    hir2mir_info_add_instr(p_info, p_new_instr);
    return p_new_instr;
}

p_mir_instr hir2mir_exp_call_gen(p_hir2mir_info p_info, p_hir_exp p_exp) {
    assert(p_exp && p_exp->kind == hir_exp_call);

    p_mir_func p_func = p_info->p_program_info->func_table + p_exp->p_func->p_sym->id;

    basic_type b_type = p_func->p_func_sym->p_type->basic;
    p_mir_vreg p_des = mir_vreg_gen(b_type, 0);

    p_mir_param_list p_m_param_list = hir2mir_param_list_gen(p_info, p_exp->p_param_list);

    p_mir_instr p_new_instr = mir_call_instr_gen(p_func, p_m_param_list, p_des);
    hir2mir_info_add_instr(p_info, p_new_instr);

    return p_new_instr;
}
