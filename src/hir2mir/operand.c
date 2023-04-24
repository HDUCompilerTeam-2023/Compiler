#include <hir2mir.h>
#include <hir/exp.h>

// 主要解决 int a[2][3]; t = a[2]; 存储 array 类型
p_mir_operand hir2mir_operand_temp_sym_array_gen(p_hir2mir_info p_info, basic_type b_type)
{
    p_mir_operand p_new_op = mir_operand_temp_sym_gen(mir_temp_sym_pointer_gen(b_type, p_info->p_func));
    return p_new_op;
}

p_mir_operand hir2mir_operand_temp_sym_basic_gen(p_hir2mir_info p_info, basic_type b_type)
{
    p_mir_operand p_new_op = mir_operand_temp_sym_gen(mir_temp_sym_basic_gen(b_type, p_info->p_func));
    return p_new_op;
}