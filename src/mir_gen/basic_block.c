
#include <mir_gen.h>
#include <mir_gen/basic_block.h>


p_mir_basic_block mir_basic_block_gen()
{
    p_mir_basic_block p_mir_block = malloc(sizeof(*p_mir_block));
    *p_mir_block = (mir_basic_block){
        .instr_list = list_head_init(&p_mir_block->instr_list),
        .block_prev = list_head_init(&p_mir_block->block_prev),
        .block_id = 0,
        .if_visited = false,
    };
    return p_mir_block;
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
size_t mir_basic_block_set_id(size_t id, p_mir_basic_block p_basic_block)
{
    if (p_basic_block->block_id) return id;
    // return 语句最后设置
    if (mir_basic_block_if_ret(p_basic_block)) return id;
    p_basic_block->block_id = ++ id;
    
    p_mir_instr p_instr = NULL;
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->instr_list){
        p_instr = list_entry(p_node, mir_instr, node);
        id = mir_instr_set_temp_var_id(id, p_instr);
    }
    if(p_instr){
        if (p_instr->irkind == mir_br) 
            return mir_basic_block_set_id(id, p_instr->mir_br.p_target);
        else if (p_instr->irkind == mir_condbr) {
            id = mir_basic_block_set_id(id, p_instr->mir_condbr.p_target_true);
            return mir_basic_block_set_id(id, p_instr->mir_condbr.p_target_false);
        }
    }
    return id;
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
    while (!list_head_alone(&p_basic_block_list->basic_block_list)) {
        p_mir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block_list->basic_block_list.p_next, mir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        mir_basic_block_drop(p_basic_block_list_node->p_basic_block);
        free(p_basic_block_list_node);
    }
    free(p_basic_block_list);
}