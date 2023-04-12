#ifndef __MIR_GEN_PARAM__
#define __MIR_GEN_PARAM__
#include <mir/param.h>

p_mir_param_list mir_param_list_init(void);
p_mir_param_list mir_param_list_add(p_mir_param_list p_param_list, p_mir_operand p_param);

void mir_param_list_drop(p_mir_param_list p_param_list);
#endif