#include <hir/exp.h>
#include <hir2mir.h>

p_mir_temp_sym hir2mir_temp_sym_pointer_gen(basic_type b_type, p_mir_func p_func) {
    p_mir_temp_sym p_temp_sym = mir_temp_sym_pointer_gen(b_type);
    mir_func_temp_sym_add(p_func, p_temp_sym);
    return p_temp_sym;
}

p_mir_temp_sym hir2mir_temp_sym_basic_gen(basic_type b_type, p_mir_func p_func) {
    p_mir_temp_sym p_temp_sym = mir_temp_sym_basic_gen(b_type);
    mir_func_temp_sym_add(p_func, p_temp_sym);
    return p_temp_sym;
}
// 主要解决 int a[2][3]; t = a[2]; 存储 array 类型
p_mir_operand hir2mir_operand_temp_sym_array_gen(p_hir2mir_info p_info, basic_type b_type) {
    p_mir_operand p_new_op = mir_operand_temp_sym_gen(hir2mir_temp_sym_pointer_gen(b_type, p_info->p_func));
    return p_new_op;
}

p_mir_operand hir2mir_operand_temp_sym_basic_gen(p_hir2mir_info p_info, basic_type b_type) {
    p_mir_operand p_new_op = mir_operand_temp_sym_gen(hir2mir_temp_sym_basic_gen(b_type, p_info->p_func));
    return p_new_op;
}