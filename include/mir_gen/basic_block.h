#ifndef __MIR_GEN_BASIC_BLOCK__
#define __MIR_GEN_BASIC_BLOCK__
#include <mir/basic_block.h>
p_mir_basic_block mir_basic_block_gen();
p_mir_basic_block_list mir_basic_block_list_gen(void);
p_mir_basic_block_list mir_basic_block_list_add(p_mir_basic_block_list p_basic_block_list, p_mir_basic_block p_basic_block);

p_mir_basic_block mir_basic_block_addinstr(p_mir_basic_block p_basic_block, p_mir_instr p_instr);

void mir_basic_block_visited_init(p_mir_basic_block p_basic_block);// 初始化图的访问标记

bool mir_basic_block_if_ret(p_mir_basic_block p_basic_block);//  是否是 ret 出口块
size_t mir_basic_block_set_id(size_t id, p_mir_basic_block p_basic_block); // 对basic_block 及所有后继 block 设置id

void mir_basic_block_drop(p_mir_basic_block p_basic_block);
void mir_basic_block_list_drop(p_mir_basic_block_list p_basic_block_list);

#endif