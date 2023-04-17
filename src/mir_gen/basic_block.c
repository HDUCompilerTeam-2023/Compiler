
#include <mir_gen.h>
#include <mir_gen/basic_block.h>

p_mir_basic_block mir_basic_block_gen(size_t block_id)
{
    p_mir_basic_block p_mir_block = malloc(sizeof(*p_mir_block));
    *p_mir_block = (mir_basic_block){
        .instr_list = list_init_head(&p_mir_block->instr_list),
        .block_prev = list_init_head(&p_mir_block->block_prev),
        .block_id = block_id,
    };
    return p_mir_block;
}

p_mir_basic_block mir_basic_block_addinstr(p_mir_basic_block p_basic_block, p_mir_instr p_instr)
{
    list_add_prev(&p_instr->node, &p_basic_block->instr_list);
    return p_basic_block;
}
void mir_basic_block_drop(p_mir_basic_block p_basic_block)
{
    assert(p_basic_block);
    p_mir_instr p_instr;
    while (!list_head_alone(&p_basic_block->instr_list)) {
        p_instr = list_entry(p_basic_block->instr_list.p_next, mir_instr, node);
        list_del(&p_instr->node);
        if (p_instr->irkind == mir_br ) {
            if (p_instr->mir_br.p_target->block_id > p_basic_block->block_id)
                mir_basic_block_drop(p_instr->mir_br.p_target);
        }
        if (p_instr->irkind == mir_condbr) {
            if (p_instr->mir_condbr.p_target_true->block_id > p_basic_block->block_id)
                mir_basic_block_drop(p_instr->mir_condbr.p_target_true);
            if (p_instr->mir_condbr.p_target_false->block_id > p_basic_block->block_id)
                mir_basic_block_drop(p_instr->mir_condbr.p_target_false);
        }
        mir_instr_drop(p_instr);
    }
    free(p_basic_block);
}