#ifndef __MIR_BASIC_BLOCK
#define __MIR_BASIC_BLOCK

#include "mir.h"

struct mir_basic_block{
    size_t block_id;
    list_head instr_list;

    list_head prev_basic_block_list;
    list_head node;
};

struct mir_basic_block_list_node{
    p_mir_basic_block p_basic_block;
    list_head node;
};

#endif