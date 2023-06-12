#ifndef __IR_PRINT_BASIC_BLOCK__
#define __IR_PRINT_BASIC_BLOCK__
#include <ir.h>
void ir_basic_block_print(p_ir_basic_block p_basic_block);
void ir_basic_block_branch_target_print(p_ir_basic_block_branch_target p_branch_call);

void ir_basic_block_dom_info_print(p_ir_basic_block p_basic_block, size_t depth);
#endif
