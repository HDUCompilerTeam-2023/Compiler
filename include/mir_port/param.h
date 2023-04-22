#ifndef __MIR_PORT_PARAM__
#define __MIR_PORT_PARAM__
#include <mir.h>

// 获取参数的操作数
p_mir_operand mir_param_get_operand(p_mir_param p_param);
p_list_head mir_param_list_get_head(p_mir_param_list p_mir_param_list);

#endif