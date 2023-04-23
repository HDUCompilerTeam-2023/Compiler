
#include "mir_gen/basic_block.h" // 头文件包含还需考虑
#include <mir_print.h>
#include <mir/basic_block.h>
#include <mir/instr.h>

#include <stdio.h>
void mir_basic_block_print(p_mir_basic_block p_basic_block)
{
    assert(p_basic_block);

    printf("b%ld: ", p_basic_block->block_id);
    if (!list_head_alone(&p_basic_block->prev_basic_block_list)) {
        printf("                        ; preds = ");
        p_list_head p_node;
        list_for_each(p_node, &p_basic_block->prev_basic_block_list){
            size_t id = list_entry(p_node, mir_basic_block_list_node, node)->p_basic_block->block_id;
            printf("b%ld", id);
            if(p_node->p_next != &p_basic_block->prev_basic_block_list)
                printf(", ");
        }
    }
    printf("\n");

    p_list_head p_node;
    p_mir_instr p_instr = NULL;
    list_for_each(p_node, &p_basic_block->instr_list){
        p_instr = list_entry(p_node, mir_instr, node);
        mir_instr_print(p_instr);
    }
}
