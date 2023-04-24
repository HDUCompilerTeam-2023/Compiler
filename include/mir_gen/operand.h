#ifndef __MIR_GEN_OPERAND__
#define __MIR_GEN_OPERAND__

#include <mir/operand.h>
p_mir_operand mir_operand_int_gen(int intconst);
p_mir_operand mir_operand_float_gen(float floatconst);
p_mir_operand mir_operand_void_gen(void);

p_mir_operand mir_operand_declared_sym_gen(p_symbol_sym p_mir_sym);

p_mir_operand mir_operand_temp_sym_gen(p_mir_temp_sym p_temp_sym);

basic_type mir_operand_get_basic_type(p_mir_operand p_operand);


p_mir_operand mir_operand_copy(p_mir_operand p_operand);

// 设置 临时变量 id 返回下一个 id
size_t mir_operand_set_temp_var_id(size_t id, p_mir_operand p_operand);

void mir_operand_drop(p_mir_operand p_operand);
#endif