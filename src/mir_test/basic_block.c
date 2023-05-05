#include <mir_test.h>

#include <mir/basic_block.h>

#include <util/list.h>

memory_type mir_basic_block_test(p_list_head p_list, memory_stack **global_stack, memory_stack **top_stack) {
    p_mir_basic_block p_basic_block = list_entry(p_list->p_next, mir_basic_block, node);
    assert(p_basic_block);
    p_list_head p_node = &p_basic_block->instr_list;
    return mir_instr_test(p_node, &(*global_stack), &(*top_stack));
}