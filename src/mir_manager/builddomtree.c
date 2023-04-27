#include <mir_manager/builddomtree.h>
#include <mir_port/basic_block.h>

void cfg_build_dom_tree_info_gen(cfg_build_dom_tree_info dom_info[], p_mir_basic_block p_basic_block, size_t block_num, size_t parent, size_t current_num) {
    dom_info[current_num] = (cfg_build_dom_tree_info) {
        .p_basic_block = p_basic_block,
        .p_semi_block_list = malloc(sizeof(size_t) * (block_num - current_num)),
        .current_num_semi = 0,
        .parent = parent,
        .ancestor = -1,
    };
    p_basic_block->dfn_id = current_num;
}

size_t init_dfs_sequence(cfg_build_dom_tree_info dfs_seq[], size_t block_num, size_t current_num, size_t parent, p_mir_basic_block p_entry) {
    if (p_entry->if_visited) return current_num;
    p_entry->if_visited = true;
    size_t dfn_id = current_num;
    cfg_build_dom_tree_info_gen(dfs_seq, p_entry, block_num, parent, dfn_id);
    current_num++;

    p_mir_basic_block p_true_block = mir_basic_block_get_true(p_entry);
    p_mir_basic_block p_false_block = mir_basic_block_get_false(p_entry);

    if (p_true_block)
        current_num = init_dfs_sequence(dfs_seq, block_num, current_num, dfn_id, p_true_block);
    if (p_false_block)
        current_num = init_dfs_sequence(dfs_seq, block_num, current_num, dfn_id, p_false_block);
    return current_num;
}

// 带权并查集的查找
static inline size_t ancestor_with_lowest_semi(cfg_build_dom_tree_info dfs_seq[], size_t node) {
    size_t a = (dfs_seq + node)->ancestor;
    if ((dfs_seq + a)->ancestor != -1) { // 不能找祖先的 ancestor
        size_t ancestor_with_min_semi_id = ancestor_with_lowest_semi(dfs_seq, a);
        (dfs_seq + node)->ancestor = (dfs_seq + a)->ancestor;
        if ((dfs_seq + ancestor_with_min_semi_id)->min_semi_id < (dfs_seq + node)->min_semi_id)
            (dfs_seq + node)->min_semi_id = (dfs_seq + ancestor_with_min_semi_id)->min_semi_id;
    }
    return (dfs_seq + node)->min_semi_id;
}

// 必须保证图连通， 否则会出错
void mir_cfg_set_func_dom(p_mir_func p_func) {
    if (list_head_alone(&p_func->entry_block)) return;
    size_t block_num = list_entry(p_func->entry_block.p_prev, mir_basic_block, node)->block_id + 1;
    p_cfg_build_dom_tree_info dfs_seq = malloc(block_num * sizeof(*dfs_seq));
    // 初始化 dfs 序
    mir_basic_block_init_visited(p_func);
    p_mir_basic_block p_entry = list_entry(p_func->entry_block.p_next, mir_basic_block, node);
    init_dfs_sequence(dfs_seq, block_num, 0, 0, p_entry);

    for (size_t i = block_num - 1; i >= 1; i--) {
        p_cfg_build_dom_tree_info p_info = dfs_seq + i;
        p_list_head p_node;
        size_t min_semi_id = p_info->parent;
        // 计算半支配点
        list_for_each(p_node, &p_info->p_basic_block->prev_basic_block_list) {
            p_mir_basic_block p_prev = list_entry(p_node, mir_basic_block_list_node, node)->p_basic_block;
            if (p_prev->dfn_id <= i) // 若是真祖先取最小的
                min_semi_id = min_semi_id > p_prev->dfn_id ? p_prev->dfn_id : min_semi_id;
            else {
                size_t prev_with_min_semi_id = ancestor_with_lowest_semi(dfs_seq, p_prev->dfn_id);
                size_t prev_min_semi_id = (dfs_seq + prev_with_min_semi_id)->semi_block;
                min_semi_id = min_semi_id > prev_min_semi_id ? prev_min_semi_id : min_semi_id;
            }
        }
        p_info->semi_block = min_semi_id;
        // 现在这个点到半支配点的路径上的点的半支配点都不明确，需要暂存直到将此半支配点加入树中再计算支配点
        (dfs_seq + min_semi_id)->p_semi_block_list[(dfs_seq + min_semi_id)->current_num_semi++] = i;
        // 将父节点加入当前树中， 并求所有以父节点为半支配点的节点的支配点, 此时从父节点到半支配儿子的路径上的点的半支配点都已确定
        p_info->ancestor = p_info->parent; // 既是节点 i 生成树上的父亲 又是半支配树上的父亲
        p_info->min_semi_id = i;
        p_cfg_build_dom_tree_info p_parent_info = dfs_seq + p_info->parent;
        for (size_t j = 0; j < p_parent_info->current_num_semi; j++) {
            size_t semi_son = p_parent_info->p_semi_block_list[j];
            // 计算从半支配树上的父节点到子节点对应的 dfs 树上的路径所有节点的半支配点的最小值

            min_semi_id = ancestor_with_lowest_semi(dfs_seq, semi_son);
            if ((dfs_seq + min_semi_id)->semi_block == p_info->parent) // 半支配点与支配点重合
                mir_basic_block_add_dom_son(p_parent_info->p_basic_block, (dfs_seq + semi_son)->p_basic_block);
            //(dfs_seq + semi_son)->p_basic_block->p_dom_parent = p_parent_info->p_basic_block;
            else // 否则直接支配点为最小的半支配点节点对应的支配点， 此时支配点可能未知， 需要暂存这个点，之后计算
                (dfs_seq + semi_son)->p_basic_block->p_dom_parent = (dfs_seq + min_semi_id)->p_basic_block;
        }
        // 置空 半支配子节点， 否则会影响之后的生成
        p_parent_info->current_num_semi = 0;
    }

    // 回填支配点
    for (size_t i = 1; i < block_num; i++) {
        p_cfg_build_dom_tree_info p_info = dfs_seq + i;
        // 若该节点的半支配点与支配点相同， 说明之前已经计算出了支配点（第二种情况假定的支配点必定在半支配点之下）
        if (p_info->p_basic_block->p_dom_parent != (dfs_seq + p_info->semi_block)->p_basic_block)
            mir_basic_block_add_dom_son(p_info->p_basic_block->p_dom_parent->p_dom_parent, p_info->p_basic_block);
    }
    for (size_t i = 0; i < block_num; i++) {
        cfg_build_dom_tree_info_drop(dfs_seq + i);
    }
    free(dfs_seq);
}

void mir_cfg_set_program_dom(p_mir_program p_program) {
    for (size_t i = 0; i < p_program->func_cnt; i++)
        mir_cfg_set_func_dom(p_program->func_table + i);
}

void cfg_build_dom_tree_info_drop(p_cfg_build_dom_tree_info p_dom_info) {
    free(p_dom_info->p_semi_block_list);
}