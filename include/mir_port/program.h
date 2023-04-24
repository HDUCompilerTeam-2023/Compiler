#ifndef __MIR_PORT_PROGRAM__
#define __MIR_PORT_PROGRAM__
#include <mir.h>
// 获取全局 变量 list
p_list_head mir_program_get_global_list(p_mir_program p_program);
p_mir_func mir_program_get_func_table(p_mir_program p_program);

#endif