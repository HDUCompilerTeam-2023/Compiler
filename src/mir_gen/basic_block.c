
#include <mir_gen.h>
#include <mir_gen/basic_block.h>


p_mir_basic_block mir_basic_block_gen()
{
    p_mir_basic_block p_mir_block = malloc(sizeof(*p_mir_block));
    *p_mir_block = (mir_basic_block){
        .instr_list = list_head_init(&p_mir_block->instr_list),
        .prev_basic_block_list = list_head_init(&p_mir_block->prev_basic_block_list),
        .block_id = 0,
        .node = list_head_init(&p_mir_block->node),
    };
    return p_mir_block;
}
// 插入前驱节点列表
p_mir_basic_block mir_basic_block_add_prev(p_mir_basic_block p_prev, p_mir_basic_block p_next)
{
    p_mir_basic_block_list_node node = malloc(sizeof(*node));
    *node = (mir_basic_block_list_node){
        .p_basic_block = p_prev,
        .node = list_head_init(&node->node),
    };
    list_add_prev(&node->node, &p_next->prev_basic_block_list);
    return p_next;
}


p_mir_basic_block mir_basic_block_addinstr(p_mir_basic_block p_basic_block, p_mir_instr p_instr)
{
    list_add_prev(&p_instr->node, &p_basic_block->instr_list);
    return p_basic_block;
}



void mir_basic_block_drop(p_mir_basic_block p_basic_block)
{
    assert(p_basic_block);
    while (!list_head_alone(&p_basic_block->instr_list)) {
        p_mir_instr p_instr = list_entry(p_basic_block->instr_list.p_next, mir_instr, node);
        list_del(&p_instr->node);
        mir_instr_drop(p_instr);
    }
    while (!list_head_alone(&p_basic_block->prev_basic_block_list)) {
        p_mir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block->prev_basic_block_list.p_next, mir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        free(p_basic_block_list_node);
    }
    free(p_basic_block);
}
