
#include <mir_gen.h>
#include <mir_gen/basic_block.h>


p_mir_basic_block mir_basic_block_gen()
{
    p_mir_basic_block p_mir_block = malloc(sizeof(*p_mir_block));
    *p_mir_block = (mir_basic_block){
        .instr_list = list_head_init(&p_mir_block->instr_list),
        .p_prev_block_list = mir_basic_block_list_gen(),
        .block_id = 0,
        .if_visited = false,
    };
    return p_mir_block;
}
// 插入前驱节点列表
p_mir_basic_block mir_basic_block_add_prev(p_mir_basic_block p_prev, p_mir_basic_block p_next)
{
    mir_basic_block_list_add(p_next->p_prev_block_list, p_prev);
    return p_next;
}

p_mir_basic_block_list mir_basic_block_list_gen(void)
{
    p_mir_basic_block_list p_basic_block_list = malloc(sizeof(*p_basic_block_list));
    p_basic_block_list->basic_block_list = list_head_init(&p_basic_block_list->basic_block_list);
    return p_basic_block_list;
}

p_mir_basic_block_list mir_basic_block_list_add(p_mir_basic_block_list p_basic_block_list,p_mir_basic_block p_basic_block)
{
    assert(p_basic_block);
    p_mir_basic_block_list_node p_basic_block_list_node = malloc(sizeof(*p_basic_block_list_node));
    *p_basic_block_list_node = (mir_basic_block_list_node){
        .p_basic_block = p_basic_block,
        .node = list_head_init(&p_basic_block_list_node->node),
    };
    list_add_prev(&p_basic_block_list_node->node, &p_basic_block_list->basic_block_list);
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
    p_mir_instr p_last_instr = list_entry(p_basic_block->instr_list.p_prev, mir_instr, node);
    if (p_last_instr->irkind == mir_br) 
        mir_basic_block_visited_init(p_last_instr->mir_br.p_target);
    else if (p_last_instr->irkind == mir_condbr) {
        mir_basic_block_visited_init(p_last_instr->mir_condbr.p_target_false);
        mir_basic_block_visited_init(p_last_instr->mir_condbr.p_target_true);
    }
}

bool mir_basic_block_if_ret(p_mir_basic_block p_basic_block)
{
    p_mir_instr p_last_instr = list_entry(p_basic_block->instr_list.p_prev, mir_instr, node);
    return p_last_instr->irkind == mir_ret;
}

 // 对basic_block 及所有后继 block 设置id, 返回最后的 id 值
#include <stdio.h>
size_t mir_basic_block_set_id(size_t b_id, p_mir_basic_block p_basic_block)
{
    if (p_basic_block->block_id) return b_id;
    // return 语句最后设置
    if (mir_basic_block_if_ret(p_basic_block)) return b_id;
    p_basic_block->block_id = ++ b_id;
    
    p_mir_instr p_last_instr = list_entry(p_basic_block->instr_list.p_prev, mir_instr, node);
    if(p_last_instr){
        if (p_last_instr->irkind == mir_br) 
            return mir_basic_block_set_id(b_id, p_last_instr->mir_br.p_target);
        else if (p_last_instr->irkind == mir_condbr) {
            b_id = mir_basic_block_set_id(b_id, p_last_instr->mir_condbr.p_target_true);
            return mir_basic_block_set_id(b_id, p_last_instr->mir_condbr.p_target_false);
        }
    }
    return b_id;
}



void mir_basic_block_drop(p_mir_basic_block p_basic_block)
{
    assert(p_basic_block);
    while (!list_head_alone(&p_basic_block->instr_list)) {
        p_mir_instr p_instr = list_entry(p_basic_block->instr_list.p_next, mir_instr, node);
        list_del(&p_instr->node);
        mir_instr_drop(p_instr);
    }
    while (!list_head_alone(&p_basic_block->p_prev_block_list->basic_block_list)) {
        p_mir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block->p_prev_block_list->basic_block_list.p_next, mir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        free(p_basic_block_list_node);
    }
    free(p_basic_block->p_prev_block_list);
    free(p_basic_block);
}

void mir_basic_block_list_drop(p_mir_basic_block_list p_basic_block_list)
{
    assert(p_basic_block_list);
    while (!list_head_alone(&p_basic_block_list->basic_block_list)) {
        p_mir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block_list->basic_block_list.p_next, mir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        mir_basic_block_drop(p_basic_block_list_node->p_basic_block);
        free(p_basic_block_list_node);
    }
    free(p_basic_block_list);
}