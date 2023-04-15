#ifndef __MIR_BASIC_BLOCK
#define __MIR_BASIC_BLOCK

#include "mir.h"

struct mir_basic_block{
    list_head instr_list;
    list_head block_prev;
    size_t block_id;
};

#endif