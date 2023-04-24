#include <mir_port/instr.h>
#include <mir/instr.h>

p_mir_operand mir_instr_binary_get_src1(p_mir_instr p_instr)
{
    return p_instr->mir_binary.p_src1;
}
p_mir_operand mir_instr_binary_get_src2(p_mir_instr p_instr)
{
    return p_instr->mir_binary.p_src2;
}
p_mir_operand mir_instr_binary_get_des(p_mir_instr p_instr)
{
    return p_instr->mir_binary.p_des;
}
p_mir_operand mir_instr_unary_get_src(p_mir_instr p_instr)
{
    return p_instr->mir_unary.p_src;
}
p_mir_operand mir_instr_unary_get_des(p_mir_instr p_instr)
{
    return p_instr->mir_unary.p_des;
}
p_mir_operand mir_instr_ret_get_src(p_mir_instr p_instr)
{
    return p_instr->mir_ret.p_ret;
}
p_mir_func mir_instr_call_get_func(p_mir_instr p_instr)
{
    return p_instr->mir_call.p_func;
}
p_mir_operand mir_instr_call_get_des(p_mir_instr p_instr)
{
    return p_instr->mir_call.p_des;
}
p_mir_param_list mir_instr_call_get_param(p_mir_instr p_instr)
{
    return p_instr->mir_call.p_param_list;
}
p_mir_operand mir_instr_array_get_array(p_mir_instr p_instr)
{
    return p_instr->mir_array.p_array;
}
p_mir_operand mir_instr_array_get_offset(p_mir_instr p_instr)
{
    return p_instr->mir_array.p_offset;
}
p_mir_operand mir_instr_array_get_des(p_mir_instr p_instr)
{
    return p_instr->mir_array.p_des;
}

p_mir_operand mir_instr_array_assign_get_array(p_mir_instr p_instr)
{
    return p_instr->mir_array_assign.p_array;
}
p_mir_operand mir_instr_array_assign_get_offset(p_mir_instr p_instr)
{
    return p_instr->mir_array_assign.p_offset;
}
p_mir_operand mir_instr_array_assign_get_src(p_mir_instr p_instr)
{
    return p_instr->mir_array_assign.p_src;
}
p_mir_basic_block mir_instr_br_get_target(p_mir_instr p_instr)
{
    return p_instr->mir_br.p_target;
}
p_mir_operand mir_instr_condbr_get_cond(p_mir_instr p_instr)
{
    return p_instr->mir_condbr.p_cond;
}
p_mir_basic_block mir_instr_condbr_get_target_true(p_mir_instr p_instr)
{
    return p_instr->mir_condbr.p_target_true;
}
p_mir_basic_block mir_instr_condbr_get_target_false(p_mir_instr p_instr)
{
    return p_instr->mir_condbr.p_target_false;
}

mir_instr_type mir_instr_get_kind(p_mir_instr p_instr)
{
    return p_instr->irkind;
}