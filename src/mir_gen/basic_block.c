
#include <mir_gen.h>
#include <mir_gen/basic_block.h>

p_mir_basic_block mir_basic_block_gen(size_t block_id)
{
    p_mir_basic_block p_mir_block = malloc(sizeof(*p_mir_block));
    *p_mir_block = (mir_basic_block){
        .instr_list = list_head_init(&p_mir_block->instr_list),
        .block_prev = list_head_init(&p_mir_block->block_prev),
        .block_id = block_id,
        .node = list_head_init(&p_mir_block->node),
        .if_visited = false,
    };
    return p_mir_block;
}

p_mir_basic_block_list mir_basic_block_list_gen(void)
{
    p_mir_basic_block_list p_basic_block_list = malloc(sizeof(*p_basic_block_list));
    p_basic_block_list->basic_block = list_head_init(&p_basic_block_list->basic_block);
    return p_basic_block_list;
}

p_mir_basic_block_list mir_basic_block_list_add(p_mir_basic_block_list p_basic_block_list,p_mir_basic_block p_basic_block)
{
    assert(p_basic_block);
    list_add_prev(&p_basic_block->node, &p_basic_block_list->basic_block);
    return p_basic_block_list;
}

p_mir_basic_block mir_basic_block_addinstr(p_mir_basic_block p_basic_block, p_mir_instr p_instr)
{
    list_add_prev(&p_instr->node, &p_basic_block->instr_list);
    return p_basic_block;
}

// 初始化图的访问标记， 图上的标记为全 true 或全 flase
void mir_basic_block_visited_init(p_mir_basic_block p_basic_block)
{
    if (!p_basic_block->if_visited) 
        return;
    p_basic_block->if_visited = false;
    p_mir_instr p_last_instr = list_entry(p_basic_block->node.p_prev, mir_instr, node);
    if (p_last_instr->irkind == mir_br) 
        mir_basic_block_visited_init(p_last_instr->mir_br.p_target);
    else if (p_last_instr->irkind == mir_condbr) {
        mir_basic_block_visited_init(p_last_instr->mir_condbr.p_target_false);
        mir_basic_block_visited_init(p_last_instr->mir_condbr.p_target_true);
    }
}

void mir_basic_block_drop(p_mir_basic_block p_basic_block)
{
    assert(p_basic_block);
    while (!list_head_alone(&p_basic_block->instr_list)) {
        p_mir_instr p_instr = list_entry(p_basic_block->instr_list.p_next, mir_instr, node);
        list_del(&p_instr->node);
        mir_instr_drop(p_instr);
    }
    free(p_basic_block);
}

void mir_basic_block_list_drop(p_mir_basic_block_list p_basic_block_list)
{
    assert(p_basic_block_list);
    while (!list_head_alone(&p_basic_block_list->basic_block)) {
        p_mir_basic_block p_basic_block = list_entry(p_basic_block_list->basic_block.p_next, mir_basic_block, node);
        list_del(&p_basic_block->node);
        mir_basic_block_drop(p_basic_block);
    }
    free(p_basic_block_list);
}