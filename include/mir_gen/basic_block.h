#ifndef __MIR_GEN_BASIC_BLOCK__
#define __MIR_GEN_BASIC_BLOCK__
#include <mir/basic_block.h>
p_mir_basic_block mir_basic_block_gen();
p_mir_basic_block mir_basic_block_add_prev(p_mir_basic_block p_prev, p_mir_basic_block p_next);

p_mir_basic_block mir_basic_block_addinstr(p_mir_basic_block p_basic_block, p_mir_instr p_instr);

p_mir_basic_block_call mir_basic_block_call_gen(p_mir_basic_block p_block);
void mir_basic_block_call_add_param(p_mir_basic_block_call p_mir_basic_block_call, p_mir_operand p_operand);

void mir_basic_block_init_visited(p_mir_func p_func);

void mir_basic_block_add_dom_son(p_mir_basic_block p_basic_block, p_mir_basic_block p_son);
void mir_basic_block_add_param(p_mir_basic_block p_basic_block, p_mir_operand p_operand);

void mir_basic_block_drop(p_mir_basic_block p_basic_block);
void mir_basic_block_call_drop(p_mir_basic_block_call p_block_call);
#endif