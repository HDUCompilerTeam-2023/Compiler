#include <mir_gen.h>
#include <mir_gen/basic_block.h>

static size_t num;
p_mir_basic_block mir_basic_block_gen(void)
{
    p_mir_basic_block p_mir_block = malloc(sizeof(*p_mir_block));
    *p_mir_block = (mir_basic_block){
        .instr_list = list_init_head(&p_mir_block->instr_list),
    };
    num = 0;// 初始化指令编号
    return p_mir_block;
}

p_mir_basic_block mir_basic_block_addinstr(p_mir_basic_block p_basic_block, p_mir_instr p_instr)
{
    p_instr->index  = num ++;
    list_add_prev(&p_instr->node, &p_basic_block->instr_list);
    return p_basic_block;
}
void mir_basic_block_drop(p_mir_basic_block p_basic_block)
{
    assert(p_basic_block);
    while (!list_head_alone(&p_basic_block->instr_list)) {
        p_mir_instr p_instr = list_entry(&p_basic_block->instr_list.p_next, mir_instr, node);
        list_del(&p_instr->node);
        mir_instr_drop(p_instr);
    }
    free(p_basic_block);
    num = 0;
}