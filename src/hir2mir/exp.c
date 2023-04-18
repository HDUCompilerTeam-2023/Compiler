
#include <hir2mir.h>
#include <hir/exp.h>

static inline basic_type mir_operand_get_basic_type(p_mir_operand p_operand)
{
    if (p_operand->kind == immedicate_val || p_operand->kind == temp_var_basic) 
        return p_operand->b_type;
    else
    {
        assert(p_operand->p_type->kind != type_arrary);
        return p_operand->p_type->basic;
    }
}

// 根据 p_exp 生成指令并返回最后一条指令的左值
p_mir_operand hir2mir_exp_get_operand(p_hir2mir_info p_info, p_hir_exp p_exp)
{
    p_mir_operand p_operand;
    switch (p_exp->kind) {
        case hir_exp_num:
            return hir2mir_operand_num_gen(p_info, p_exp);
        case hir_exp_val:
            p_operand = hir2mir_operand_declared_sym_gen(p_info, p_exp->p_sym);
            if (p_exp->p_offset) // 若是数组元素赋值 需要新增一条语句将数组元素赋值给临时变量
            {
                p_mir_operand p_offset = hir2mir_exp_get_operand(p_info, p_exp->p_offset);
                p_mir_operand p_temp_des = hir2mir_operand_temp_sym_array_gen(p_info, p_exp->p_type);
                p_mir_instr p_instr = mir_array_instr_gen(p_operand, p_offset, p_temp_des);
                mir_basic_block_addinstr(p_info->p_current_basic_block, p_instr);
                return mir_instr_get_des(p_instr);
            }
            else        
                return p_operand;
        default:
            return mir_instr_get_des(hir2mir_exp_gen(p_info, p_exp));
    }
}

p_mir_instr hir2mir_exp_gen(p_hir2mir_info p_info, p_hir_exp p_exp)
{
    assert(p_exp);
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
            case hir_exp_op_bool_or:
            case hir_exp_op_bool_and:
                return hir2mir_exp_lexec_gen(p_info, p_exp);
        }
        case hir_exp_call:
            return hir2mir_exp_call_gen(p_info, p_exp);
        default:
            assert(0);
    }
}
// 经过常量传播后若非倒数第二层节点 左操作数必定至少存在一个变量
// 而倒数第二层节点两个操作数必定存在一个变量
p_mir_instr hir2mir_exp_exec_gen(p_hir2mir_info p_info, p_hir_exp p_exp)
{
    assert(p_exp && p_exp->p_src_1 && p_exp->p_src_2);
    p_mir_operand p_operand1 = hir2mir_exp_get_operand(p_info, p_exp->p_src_1);
    p_mir_operand p_operand2 = hir2mir_exp_get_operand(p_info, p_exp->p_src_2);

    basic_type b_type1 = mir_operand_get_basic_type(p_operand1);
    basic_type b_type2 = mir_operand_get_basic_type(p_operand2);
    
    // TODO: 类型转换
    if (b_type1 == type_float && b_type2 == type_int) {
           
    }
    p_mir_operand p_temp_des;
    p_temp_des = hir2mir_operand_temp_sym_basic_gen(p_info, b_type1);

    p_mir_instr p_new_instr = NULL;
    mir_instr_type mir_instr_kind;
    switch (p_exp->op) {
        case hir_exp_op_add:
            mir_instr_kind = mir_add_op;
            break;
        case hir_exp_op_sub:
            mir_instr_kind = mir_sub_op;
            break;
        case hir_exp_op_mul:
            mir_instr_kind = mir_mul_op;
            break;
        case hir_exp_op_div:
            mir_instr_kind = mir_div_op;
            break;
        case hir_exp_op_mod:
            mir_instr_kind = mir_div_op;
            break;
        case hir_exp_op_eq:
            mir_instr_kind = mir_eq_op;
            p_temp_des->b_type = type_int;
            break;
        case hir_exp_op_g:
            mir_instr_kind = mir_g_op;
            p_temp_des->b_type = type_int;
            break;
        case hir_exp_op_geq:
            mir_instr_kind = mir_geq_op;
            p_temp_des->b_type = type_int;
            break;
        case hir_exp_op_l:
            mir_instr_kind = mir_l_op;
            p_temp_des->b_type = type_int;
            break;
        case hir_exp_op_leq:
            mir_instr_kind = mir_leq_op;
            p_temp_des->b_type = type_int;
            break;
        default:
            assert(0);
    }
    p_new_instr = mir_binary_instr_gen(mir_instr_kind, p_operand1, p_operand2, p_temp_des);
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_new_instr);
    return p_new_instr;

}

p_mir_instr hir2mir_exp_uexec_gen(p_hir2mir_info p_info, p_hir_exp p_exp)
{
    assert(p_exp && p_exp->p_src_1);
    p_mir_operand p_operand = hir2mir_exp_get_operand(p_info, p_exp->p_src_1);
    p_mir_operand p_temp_des = NULL;
    p_mir_instr p_new_instr = NULL;
    switch (p_exp->op) {
        case hir_exp_op_bool_not:
        // 需要转换为 int 型
            p_temp_des = hir2mir_operand_temp_sym_basic_gen(p_info, type_int);
            p_new_instr = mir_unary_instr_gen(mir_not_op, p_operand, p_temp_des);
            break;
        case hir_exp_op_minus:
            p_temp_des = hir2mir_operand_temp_sym_basic_gen(p_info, mir_operand_get_basic_type(p_operand));
            p_new_instr = mir_unary_instr_gen(mir_minus_op, p_operand, p_temp_des);
            break;
        default:
            assert(0);
    }
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_new_instr);
    return p_new_instr;
}

p_mir_instr hir2mir_exp_lexec_gen(p_hir2mir_info p_info, p_hir_exp p_exp)
{
    assert(p_exp && p_exp->p_src_1 && p_exp->p_src_2);
    //p_mir_instr p_new_instr = NULL; 
    //p_mir_basic_block p_current_basic_block = p_info->p_current_basic_block;
    // 短路求值
    if (p_exp->op == hir_exp_op_bool_and) {

    }
    else if (p_exp->op == hir_exp_op_bool_or){
        
    }
    return hir2mir_exp_gen(p_info, p_exp->p_src_1);
}

p_mir_instr hir2mir_exp_assign_gen(p_hir2mir_info p_info, p_hir_exp p_exp)
{
    assert(p_exp );
    assert(p_exp->p_src_1 && p_exp->p_src_2);
    assert(p_exp->p_src_1->kind == hir_exp_val);

    p_mir_instr p_new_instr = NULL;
    p_mir_operand p_des = hir2mir_operand_declared_sym_gen(p_info, p_exp->p_src_1->p_sym);
    if (p_exp->p_src_1->p_type->kind == type_arrary) {
        p_mir_operand p_offset = hir2mir_exp_get_operand(p_info, p_exp->p_src_1->p_offset);
        p_mir_operand p_src = hir2mir_exp_get_operand(p_info, p_exp->p_src_2);
        p_new_instr = mir_array_assign_instr_gen(p_des, p_offset, p_src);
    }
    else {
        p_mir_operand p_operand = hir2mir_exp_get_operand(p_info, p_exp->p_src_2);
        p_new_instr = mir_unary_instr_gen(mir_val_assign, p_operand, p_des);
    }
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_new_instr);
    return p_new_instr;
}

p_mir_instr hir2mir_exp_call_gen(p_hir2mir_info p_info, p_hir_exp p_exp)
{
    assert(p_exp && p_exp->kind == hir_exp_call);

    p_mir_operand p_func = hir2mir_operand_declared_sym_gen(p_info, p_exp->p_sym);

    p_mir_operand p_des = hir2mir_operand_temp_sym_basic_gen(p_info, p_func->p_type->basic);

    p_mir_param_list p_m_param_list = hir2mir_param_list_gen(p_info, p_exp->p_param_list);

    p_mir_instr p_new_instr = mir_call_instr_gen(p_func, p_m_param_list, p_des);
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_new_instr);
    
    return p_new_instr;
}