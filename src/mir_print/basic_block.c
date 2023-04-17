
#include <mir_print.h>
#include <mir/basic_block.h>
#include <mir/instr.h>

#include <stdio.h>
void mir_basic_block_print(p_mir_basic_block p_basic_block)
{
    assert(p_basic_block);
    printf("b%ld: \n", p_basic_block->block_id);
    p_list_head p_node;
    p_mir_instr p_instr = NULL;
    list_for_each(p_node, &p_basic_block->instr_list){
        p_instr = list_entry(p_node, mir_instr, node);
        mir_instr_print(p_instr);
    }
    if (p_instr->irkind == mir_br ) {
        if (p_instr->mir_br.p_target->block_id > p_basic_block->block_id)
            mir_basic_block_print(p_instr->mir_br.p_target);
    }
    if (p_instr->irkind == mir_condbr) {
        if (p_instr->mir_condbr.p_target_true->block_id > p_basic_block->block_id)
            mir_basic_block_print(p_instr->mir_condbr.p_target_true);
        if (p_instr->mir_condbr.p_target_false->block_id > p_basic_block->block_id)
            mir_basic_block_print(p_instr->mir_condbr.p_target_false);
    }
}