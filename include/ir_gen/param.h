#ifndef __IR_GEN_PARAM__
#define __IR_GEN_PARAM__
#include <ir/param.h>

p_ir_param_list ir_param_list_init(void);
p_ir_param_list ir_param_list_add(p_ir_param_list p_param_list, p_ir_operand p_param);

void ir_param_list_drop(p_ir_param_list p_param_list);
#endif
