#ifndef __MIR_PRINT_BASIC_BLOCK__
#define __MIR_PRINT_BASIC_BLOCK__
#include <mir.h>
void mir_basic_block_print(p_mir_basic_block p_basic_block);

void mir_basic_block_dom_info_print(p_mir_basic_block p_basic_block, size_t depth);
#endif