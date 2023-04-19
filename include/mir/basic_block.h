#ifndef __MIR_BASIC_BLOCK
#define __MIR_BASIC_BLOCK

#include "mir.h"

struct mir_basic_block{
    list_head instr_list;
    p_mir_basic_block_list p_prev_block_list;
    size_t block_id;

    bool if_visited;// 是否访问过， 用于遍历
};

struct mir_basic_block_list_node{
    p_mir_basic_block p_basic_block;
    list_head node;
};
struct mir_basic_block_list{
    list_head basic_block_list;
};

#endif