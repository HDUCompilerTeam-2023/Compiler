#include <mir_port/basic_block.h>
#include <mir/basic_block.h>
#include <mir/instr.h>

p_list_head mir_basic_block_get_instr_list_head(p_mir_basic_block p_basic_block)
{
    return &p_basic_block->instr_list;
}
p_mir_instr mir_basic_block_get_last_instr(p_mir_basic_block p_basic_block)
{
    return list_entry(p_basic_block->instr_list.p_prev, mir_instr, node);
}

size_t mir_basic_block_get_id(p_mir_basic_block p_basic_block)
{
    return p_basic_block->block_id;
}
// 返回该 基本块最后一条指令的正确跳转目标块， 没有返回 NULL
p_mir_basic_block mir_basic_block_get_true(p_mir_basic_block p_basic_block)
{
    p_mir_instr p_last_instr = mir_basic_block_get_last_instr(p_basic_block);
    if (p_last_instr->irkind == mir_br) {
        return p_last_instr->mir_br.p_target;
    }
    else if (p_last_instr->irkind ==  mir_condbr) {
        return p_last_instr->mir_condbr.p_target_true;
    }
    return NULL;
}

p_mir_basic_block mir_basic_block_get_false(p_mir_basic_block p_basic_block)
{
    p_mir_instr p_last_instr = mir_basic_block_get_last_instr(p_basic_block);
    if (p_last_instr->irkind ==  mir_condbr) {
        return p_last_instr->mir_condbr.p_target_false;
    }
    return NULL;
}