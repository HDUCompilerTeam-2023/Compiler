
#include "mir_gen/basic_block.h" // 头文件包含还需考虑
#include <mir_print.h>
#include <mir/basic_block.h>
#include <mir/instr.h>

#include <stdio.h>
void mir_basic_block_print(p_mir_basic_block p_basic_block)
{
    assert(p_basic_block);
    if (p_basic_block->if_visited) return;
    if (mir_basic_block_if_ret(p_basic_block)) return; // 最后输出return
    p_basic_block->if_visited = true;

    printf("b%ld: \n", p_basic_block->block_id);
    p_list_head p_node;
    p_mir_instr p_instr = NULL;
    list_for_each(p_node, &p_basic_block->instr_list){
        p_instr = list_entry(p_node, mir_instr, node);
        mir_instr_print(p_instr);
    }
    if(p_instr){
        if (p_instr->irkind == mir_br) 
            mir_basic_block_print(p_instr->mir_br.p_target);
        else if (p_instr->irkind == mir_condbr) {
            mir_basic_block_print(p_instr->mir_condbr.p_target_true);
            mir_basic_block_print(p_instr->mir_condbr.p_target_false);
        }
    }
}

void mir_basic_block_list_print(p_mir_basic_block_list p_basic_block_list)
{
    assert(p_basic_block_list);
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block_list->basic_block_list){
        p_mir_basic_block p_basic_block = list_entry(p_node, mir_basic_block_list_node, node)->p_basic_block;
        mir_basic_block_print(p_basic_block);
    }
}