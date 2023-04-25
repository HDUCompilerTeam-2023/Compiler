#ifndef __MIR_PORT_FUNC__
#define __MIR_PORT_FUNC__
#include <mir.h>

p_list_head mir_func_get_basic_block_entry(p_mir_func p_func);

// 获取函数 symbol 信息
p_symbol_sym mir_func_get_sym(p_mir_func p_func);
#endif