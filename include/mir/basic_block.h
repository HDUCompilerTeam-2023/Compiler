#ifndef __MIR_BASIC_BLOCK
#define __MIR_BASIC_BLOCK

#include "mir.h"
// 基本块, 之后补充
struct mir_basic_block{
    p_mir_linear_instr p_instr_list;
    // TODO: 基本块间跳转
};

#endif