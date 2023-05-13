#include <mir/basic_block.h>
#include <mir/instr.h>
#include <mir_port/basic_block.h>

p_list_head mir_basic_block_get_instr_list_head(p_mir_basic_block p_basic_block) {
    return &p_basic_block->instr_list;
}
p_mir_instr mir_basic_block_get_last_instr(p_mir_basic_block p_basic_block) {
    return list_entry(p_basic_block->instr_list.p_prev, mir_instr, node);
}

size_t mir_basic_block_get_id(p_mir_basic_block p_basic_block) {
    return p_basic_block->block_id;
}
