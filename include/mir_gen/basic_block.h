#ifndef __MIR_GEN_BASIC_BLOCK__
#define __MIR_GEN_BASIC_BLOCK__
#include <mir/basic_block.h>
//p_mir_basic_block mir_basic_block_gen(void);
p_mir_basic_block mir_basic_block_gen(p_mir_func p_func);

p_mir_basic_block mir_basic_block_addinstr(p_mir_basic_block p_basic_block, p_mir_instr p_instr);
void mir_basic_block_drop(p_mir_basic_block p_basic_block);
#endif