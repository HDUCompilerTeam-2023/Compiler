#ifndef __IR_GEN_PARAM__
#define __IR_GEN_PARAM__
#include <ir/param.h>
p_ir_param ir_param_gen(p_ir_operand p_param);
void ir_param_set_vmem(p_ir_param p_param, p_symbol_var p_vmem);
void ir_param_list_drop(p_ir_param_list p_param_list);
#endif
