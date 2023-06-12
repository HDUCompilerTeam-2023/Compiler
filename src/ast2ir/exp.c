#include <ast/exp.h>
#include <ast2ir.h>

#include <program/gen.h>
#include <symbol_gen.h>

static inline p_ir_instr ast2ir_exp_binary_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    assert(p_exp->p_src_1 && p_exp->p_src_2);
    p_ir_operand p_src_1 = ast2ir_exp_gen(p_info, p_exp->p_src_1);
    p_ir_operand p_src_2 = ast2ir_exp_gen(p_info, p_exp->p_src_2);

    p_ir_vreg p_vreg = ir_vreg_gen(symbol_type_var_gen(p_exp->p_type->basic));
    p_ir_instr p_instr = NULL;
    switch (p_exp->b_op) {
    case ast_exp_op_add:
        p_instr = ir_binary_instr_gen(ir_add_op, p_src_1, p_src_2, p_vreg);
        break;
    case ast_exp_op_sub:
        p_instr = ir_binary_instr_gen(ir_sub_op, p_src_1, p_src_2, p_vreg);
        break;
    case ast_exp_op_mul:
        p_instr = ir_binary_instr_gen(ir_mul_op, p_src_1, p_src_2, p_vreg);
        break;
    case ast_exp_op_div:
        p_instr = ir_binary_instr_gen(ir_div_op, p_src_1, p_src_2, p_vreg);
        break;
    case ast_exp_op_mod:
        p_instr = ir_binary_instr_gen(ir_mod_op, p_src_1, p_src_2, p_vreg);
        break;
    case ast_exp_op_eq:
        p_instr = ir_binary_instr_gen(ir_eq_op, p_src_1, p_src_2, p_vreg);
        break;
    case ast_exp_op_neq:
        p_instr = ir_binary_instr_gen(ir_neq_op, p_src_1, p_src_2, p_vreg);
        break;
    case ast_exp_op_l:
        p_instr = ir_binary_instr_gen(ir_l_op, p_src_1, p_src_2, p_vreg);
        break;
    case ast_exp_op_leq:
        p_instr = ir_binary_instr_gen(ir_leq_op, p_src_1, p_src_2, p_vreg);
        break;
    case ast_exp_op_g:
        p_instr = ir_binary_instr_gen(ir_g_op, p_src_1, p_src_2, p_vreg);
        break;
    case ast_exp_op_geq:
        p_instr = ir_binary_instr_gen(ir_geq_op, p_src_1, p_src_2, p_vreg);
        break;
    }
    p_exp->p_des = p_vreg;
    return p_instr;
}
static inline p_ir_instr ast2ir_exp_unary_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    assert(p_exp->p_src);
    p_ir_operand p_src = ast2ir_exp_gen(p_info, p_exp->p_src);

    p_ir_vreg p_vreg = ir_vreg_gen(symbol_type_var_gen(p_exp->p_type->basic));
    p_ir_instr p_instr = NULL;
    switch (p_exp->u_op) {
    case ast_exp_op_minus:
        p_instr = ir_unary_instr_gen(ir_minus_op, p_src, p_vreg);
        break;
    }
    p_exp->p_des = p_vreg;
    return p_instr;
}
static inline p_ir_instr ast2ir_exp_logic_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    assert(0);
    assert(p_exp->p_bool_1 && p_exp->p_bool_2);
    // p_ir_operand p_bool_1 = ast2ir_exp_gen(p_info, p_exp->p_bool_1);
    // p_ir_operand p_bool_2 = ast2ir_exp_gen(p_info, p_exp->p_bool_2);

    p_ir_vreg p_vreg = ir_vreg_gen(symbol_type_var_gen(p_exp->p_type->basic));
    p_ir_instr p_instr = NULL;
    switch (p_exp->l_op) {
    case ast_exp_op_bool_or:
        break;
    case ast_exp_op_bool_and:
        break;
    }
    p_exp->p_des = p_vreg;
    return p_instr;
}
static inline p_ir_instr ast2ir_exp_ulogic_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    assert(p_exp->p_bool);
    p_ir_operand p_bool = ast2ir_exp_gen(p_info, p_exp->p_bool);

    p_ir_vreg p_vreg = ir_vreg_gen(symbol_type_var_gen(p_exp->p_type->basic));
    p_ir_instr p_instr = NULL;
    switch (p_exp->ul_op) {
    case ast_exp_op_bool_not:
        p_instr = ir_unary_instr_gen(ir_not_op, p_bool, p_vreg);
        break;
    }
    p_exp->p_des = p_vreg;
    return p_instr;
}
static inline p_ir_operand ast2ir_exp_num_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    if (p_exp->p_type->basic == type_int) {
        return ir_operand_int_gen(p_exp->intconst);
    }
    if (p_exp->p_type->basic == type_float) {
        return ir_operand_float_gen(p_exp->floatconst);
    }
    if (p_exp->p_type->basic == type_str) {
        return ir_operand_str_gen(p_exp->p_str);
    }
    assert(0);
}
static inline p_ir_instr ast2ir_exp_gep_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    p_ir_operand p_addr = ast2ir_exp_gen(p_info, p_exp->p_addr);
    p_ir_operand p_offset = ast2ir_exp_gen(p_info, p_exp->p_offset);
    p_ir_vreg p_des = ir_vreg_gen(symbol_type_copy(p_exp->p_type));
    p_ir_instr p_gep = ir_gep_instr_gen(p_addr, p_offset, p_des, p_exp->is_element);
    p_exp->p_des = p_des;
    return p_gep;
}
static inline p_ir_instr ast2ir_exp_load_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    p_ir_operand p_ptr = ast2ir_exp_gen(p_info, p_exp->p_ptr);
    p_ir_vreg p_des = ir_vreg_gen(symbol_type_copy(p_exp->p_type));
    p_ir_instr p_load = ir_load_instr_gen(p_ptr, NULL, p_des);
    p_exp->p_des = p_des;
    return p_load;
}
static inline p_ir_instr ast2ir_exp_call_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    p_ir_param_list p_m_param_list = ast2ir_param_list_gen(p_info, p_exp->p_param_list);
    p_ir_vreg p_des = NULL;
    if (p_exp->p_func->ret_type != type_void) {
        p_des = ir_vreg_gen(symbol_type_var_gen(p_exp->p_func->ret_type));
    }
    p_ir_instr p_new_instr = ir_call_instr_gen(p_exp->p_func, p_m_param_list, p_des);
    p_exp->p_des = p_des;
    return p_new_instr;
}
static inline p_ir_instr ast2ir_exp_use_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    assert(p_exp->p_exp->p_des);
    p_exp->p_des = p_exp->p_exp->p_des;
    return NULL;
}

// 根据 p_exp 生成指令并返回最后一条指令的左值
p_ir_operand ast2ir_exp_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    if (!p_exp) return NULL;
    p_ir_instr p_instr = NULL;
    switch (p_exp->kind) {
    case ast_exp_num: // 若是常量 直接返回该常量对应的操作数
        return ast2ir_exp_num_gen(p_info, p_exp);
    case ast_exp_ptr:
        return ir_operand_addr_gen(p_exp->p_var);
    case ast_exp_gep:
        p_instr = ast2ir_exp_gep_gen(p_info, p_exp);
        break;
    case ast_exp_load:
        p_instr = ast2ir_exp_load_gen(p_info, p_exp);
        break;
    case ast_exp_binary:
        p_instr = ast2ir_exp_binary_gen(p_info, p_exp);
        break;
    case ast_exp_unary:
        p_instr = ast2ir_exp_unary_gen(p_info, p_exp);
        break;
    case ast_exp_logic:
        p_instr = ast2ir_exp_logic_gen(p_info, p_exp);
        break;
    case ast_exp_ulogic:
        p_instr = ast2ir_exp_ulogic_gen(p_info, p_exp);
        break;
    case ast_exp_call:
        p_instr = ast2ir_exp_call_gen(p_info, p_exp);
        break;
    case ast_exp_use:
        p_instr = ast2ir_exp_use_gen(p_info, p_exp);
        break;
    }
    if (p_instr) ast2ir_info_add_instr(p_info, p_instr);
    if (!p_exp->p_des) return NULL;
    return ir_operand_vreg_gen(p_exp->p_des);
}
// exp 正确则跳向 true, 错误跳向 false
p_ir_operand ast2ir_exp_cond_gen(p_ast2ir_info p_info, p_ir_basic_block p_true_block, p_ir_basic_block p_false_block, p_ast_exp p_exp) {
    assert(p_exp);
    if (p_exp->kind == ast_exp_logic && p_exp->l_op == ast_exp_op_bool_or) {
        p_ir_basic_block p_new_false_block = ir_basic_block_gen();
        // 在当前 block 生成 左边代码
        ast2ir_exp_cond_gen(p_info, p_true_block, p_new_false_block, p_exp->p_bool_1);
        // 在新block 生成右边代码， 该block 也是左边的 false block
        ast2ir_info_add_basic_block(p_info, p_new_false_block);
        ast2ir_exp_cond_gen(p_info, p_true_block, p_false_block, p_exp->p_bool_2);
    }
    else if (p_exp->kind == ast_exp_logic && p_exp->l_op == ast_exp_op_bool_and) {
        p_ir_basic_block p_new_true_block = ir_basic_block_gen();
        // 在当前 block 生成 左边代码
        ast2ir_exp_cond_gen(p_info, p_new_true_block, p_false_block, p_exp->p_bool_1);
        // 在新block 生成右边代码， 该block 也是左边的 true block
        ast2ir_info_add_basic_block(p_info, p_new_true_block);
        ast2ir_exp_cond_gen(p_info, p_true_block, p_false_block, p_exp->p_bool_2);
    }
    else if (p_exp->kind == ast_exp_ulogic && p_exp->ul_op == ast_exp_op_bool_not) {
        ast2ir_exp_cond_gen(p_info, p_false_block, p_true_block, p_exp->p_bool);
    }
    else {
        p_ir_operand p_cond = ast2ir_exp_gen(p_info, p_exp);
        ir_basic_block_set_cond(p_info->p_current_basic_block, p_cond, p_true_block, p_false_block);
    }
    return NULL;
}
