#ifndef __MIR_PORT_BASIC_BLOCK__
#define __MIR_PORT_BASIC_BLOCK__

#include <mir.h>
p_mir_instr mir_basic_block_get_last_instr(p_mir_basic_block p_basic_block);
p_list_head mir_basic_block_get_instr_list_head(p_mir_basic_block p_basic_block);

bool mir_basic_block_get_if_visited(p_mir_basic_block p_basic_block);
size_t mir_basic_block_get_id(p_mir_basic_block p_basic_block);

#endif