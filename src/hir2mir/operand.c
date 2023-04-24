#include <hir2mir.h>
#include <hir/exp.h>
// 生成 int operand 并插入到列表中
p_mir_operand hir2mir_operand_int_gen(int intconst)
{
    p_mir_operand p_new_op = mir_operand_int_gen(intconst);
    return p_new_op;
}

p_mir_operand hir2mir_operand_float_gen(float floatconst)
{
    p_mir_operand p_new_op = mir_operand_float_gen(floatconst);
    return p_new_op;
}

p_mir_operand hir2mir_operand_num_gen(p_hir_exp p_exp)
{
    assert(p_exp->kind == hir_exp_num);
    switch (p_exp->basic) {
        case type_int:
            return hir2mir_operand_int_gen( p_exp->intconst);
            break;
        case type_float:
            return hir2mir_operand_float_gen( p_exp->floatconst);
        case type_void:
        case type_str:
            assert(0);
    }
}

p_mir_operand hir2mir_operand_void_gen()
{
    p_mir_operand p_new_op = mir_operand_void_gen();
    return p_new_op;
}

p_mir_operand hir2mir_operand_declared_sym_gen(p_symbol_sym p_sym)
{
    p_mir_operand p_new_op = mir_operand_declared_sym_gen(p_sym);
    return p_new_op;
}
// 主要解决 int a[2][3]; t = a[2]; 存储 array 类型
p_mir_operand hir2mir_operand_temp_sym_array_gen(p_hir2mir_info p_info, basic_type b_type)
{
    p_mir_temp_sym p_temp_sym = mir_temp_sym_pointer_gen(p_info->temp_id ++, b_type);
    mir_func_temp_sym_add(p_info->p_func, p_temp_sym);
    p_mir_operand p_new_op = mir_operand_temp_sym_gen(p_temp_sym);
    return p_new_op;
}

p_mir_operand hir2mir_operand_temp_sym_basic_gen(p_hir2mir_info p_info, basic_type b_type)
{
    p_mir_temp_sym p_temp_sym = mir_temp_sym_basic_gen(p_info->temp_id ++, b_type);
    mir_func_temp_sym_add(p_info->p_func, p_temp_sym);
    p_mir_operand p_new_op = mir_operand_temp_sym_gen(p_temp_sym);
    return p_new_op;
}