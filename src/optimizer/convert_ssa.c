#include <optimizer/convert_ssa.h>
#include <mir_port/basic_block.h>
void convert_ssa_gen(convert_ssa *dfs_seq, size_t block_num, p_mir_basic_block p_basic_block, size_t current_num)
{
    dfs_seq[current_num] = (convert_ssa){
        .dom_frontier = bitmap_gen(block_num),
        .p_basic_block = p_basic_block,
    };
    p_basic_block->dfn_id = current_num;
    bitmap_set_empty(dfs_seq[current_num].dom_frontier);
}

size_t convert_ssa_init_dfs_sequence(convert_ssa *dfs_seq, size_t block_num, p_mir_basic_block p_entry, size_t current_num)
{
    if(p_entry->if_visited)return current_num;
    p_entry->if_visited = true;
    convert_ssa_gen(dfs_seq, block_num, p_entry, current_num);
    current_num++;

    p_mir_basic_block p_true_block = mir_basic_block_get_true(p_entry);
    p_mir_basic_block p_false_block = mir_basic_block_get_false(p_entry);

    if (p_true_block)
        current_num = convert_ssa_init_dfs_sequence(dfs_seq, block_num, p_true_block, current_num);
    if (p_false_block)
        current_num = convert_ssa_init_dfs_sequence(dfs_seq, block_num, p_false_block, current_num);
    return current_num;
}

// 根据 p_wait_block 的直接支配父亲是否是 p_convert_ssa 对应的块， 决定是否加入集合
static inline bool add_dom_element_if_need(convert_ssa *p_convert, p_mir_basic_block p_wait_block)
{
    if (p_wait_block->p_dom_parent != p_convert->p_basic_block){
        bitmap_add_element(p_convert->dom_frontier, p_wait_block->dfn_id);
        return true;
    }
    return false;
}

void convert_ssa_compute_dom_frontier(convert_ssa *dfs_seq, size_t block_num)
{
    for(size_t i = block_num - 1; i < block_num; i --){
        p_convert_ssa p_info = dfs_seq + i;
        p_mir_basic_block p_true_block = mir_basic_block_get_true(p_info->p_basic_block);
        p_mir_basic_block p_false_block = mir_basic_block_get_false(p_info->p_basic_block);
        // 计算 DF_up
        if(p_true_block)
            add_dom_element_if_need(p_info, p_true_block);
        if (p_false_block)
            add_dom_element_if_need(p_info, p_false_block);

        // 计算 DF_local
        p_list_head p_node;
        list_for_each(p_node, &p_info->p_basic_block->dom_son_list) {
            size_t son_id = list_entry(p_node, mir_basic_block_list_node, node)->p_basic_block->dfn_id;
            p_convert_ssa p_son_info = dfs_seq + son_id;
            // 遍历子节点的支配边界，其中不属于 当前块的直接支配节点的节点即为当前块的支配边界
            for(size_t i = 0; i < block_num; i ++){
                if(bitmap_if_in(p_son_info->dom_frontier, i))
                    add_dom_element_if_need(p_info, (dfs_seq + i)->p_basic_block);
            }
        }
    }
}

#include <stdio.h>
static inline void print_dom_frontier(convert_ssa *dfs_seq, size_t block_num)
{
    printf(" --- dom_frontier start---\n");
    for(size_t i = 0; i < block_num; i ++)
    {
        p_convert_ssa p_info = dfs_seq + i;
        printf("b%ld (dfn_id: %ld): ", p_info->p_basic_block->block_id, p_info->p_basic_block->dfn_id);
        bitmap_print(p_info->dom_frontier);
        printf("\n");
    }
    printf(" --- dom_frontier end---\n");
}


void convert_ssa_func(p_mir_func p_func){
    if (list_head_alone(&p_func->entry_block)) return;
    size_t block_num = list_entry(p_func->entry_block.p_prev, mir_basic_block, node)->block_id + 1;
    p_convert_ssa dfs_seq = malloc(block_num * sizeof(*dfs_seq));
    // 初始化 dfs 序
    mir_basic_block_init_visited(p_func);
    p_mir_basic_block p_entry = list_entry(p_func->entry_block.p_next, mir_basic_block, node);
    convert_ssa_init_dfs_sequence(dfs_seq, block_num, p_entry, 0);

    convert_ssa_compute_dom_frontier(dfs_seq, block_num);
    print_dom_frontier(dfs_seq, block_num);

    convert_ssa_dfs_seq_drop(dfs_seq, block_num);
}

void convert_ssa_program(p_mir_program p_program){
    for (size_t i = 0; i < p_program->func_cnt; i++)
        convert_ssa_func(p_program->func_table + i);
}

void convert_ssa_dfs_seq_drop(convert_ssa *dfs_seq, size_t block_num) {
    for (size_t i = 0; i < block_num; i++)
        bitmap_drop((dfs_seq + i)->dom_frontier);
    free(dfs_seq);
}