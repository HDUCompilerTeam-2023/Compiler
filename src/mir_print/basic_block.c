
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
}

void mir_basic_block_list_print(p_mir_basic_block_list p_basic_block_list)
{
    assert(p_basic_block_list);
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block_list->basic_block){
        p_mir_basic_block p_basic_block = list_entry(p_node, mir_basic_block, node);
        mir_basic_block_print(p_basic_block);
    }
}