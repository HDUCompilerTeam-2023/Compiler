#ifndef __IR_GEN_PARAM__
#define __IR_GEN_PARAM__
#include <ir/param.h>

p_ir_param_list ir_param_list_init(void);
p_ir_param_list ir_param_list_add(p_ir_param_list p_param_list, p_ir_operand p_param, bool is_stack_ptr);
void ir_param_set_vmem(p_ir_param p_param, p_symbol_var p_vmem);
void ir_param_list_drop(p_ir_param_list p_param_list);
#endif
