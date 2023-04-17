
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