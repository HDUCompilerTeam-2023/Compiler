#include <ir_gen.h>
#include <ir_manager/builddomtree.h>
#include <ir_print.h>
#include <program/def.h>
#include <symbol_gen/func.h>

static inline void cfg_build_dom_tree_info_gen(p_cfg_build_dom_tree_info_list p_info_list, p_ir_basic_block p_basic_block, size_t parent, size_t current_num) {
    *(p_info_list->p_base + current_num) = (cfg_build_dom_tree_info) {
        .p_basic_block = p_basic_block,
        .p_semi_block_list = malloc(sizeof(size_t) * (p_info_list->block_num - current_num)),
        .current_num_semi = 0,
        .parent = parent,
        .ancestor = -1,
    };
    p_info_list->block2dfn_id[p_basic_block->block_id] = current_num;
}
static inline void cfg_build_dom_tree_info_list_drop(p_cfg_build_dom_tree_info_list p_info_list) {
    for (size_t i = 0; i < p_info_list->block_num; i++)
        free((p_info_list->p_base + i)->p_semi_block_list);
    free(p_info_list->p_base);
    free(p_info_list->block2dfn_id);
    free(p_info_list);
}
static inline size_t init_dfs_sequence(p_cfg_build_dom_tree_info_list p_info_list, size_t current_num, size_t parent, p_ir_basic_block p_entry) {
    if (p_entry->if_visited) return current_num;
    p_entry->if_visited = true;
    size_t dfn_id = current_num;
    cfg_build_dom_tree_info_gen(p_info_list, p_entry, parent, dfn_id);
    current_num++;

    p_ir_basic_block_branch_target p_true_block = p_entry->p_branch->p_target_1;
    p_ir_basic_block_branch_target p_false_block = p_entry->p_branch->p_target_2;

    if (p_true_block)
        current_num = init_dfs_sequence(p_info_list, current_num, dfn_id, p_true_block->p_block);
    if (p_false_block)
        current_num = init_dfs_sequence(p_info_list, current_num, dfn_id, p_false_block->p_block);
    return current_num;
}

// 带权并查集的查找
static inline size_t ancestor_with_lowest_semi(p_cfg_build_dom_tree_info_list p_info_list, size_t node) {
    size_t a = (p_info_list->p_base + node)->ancestor;
    if ((p_info_list->p_base + a)->ancestor != -1) { // 不能找祖先的 ancestor
        size_t ancestor_with_min_semi_id = ancestor_with_lowest_semi(p_info_list, a);
        (p_info_list->p_base + node)->ancestor = (p_info_list->p_base + a)->ancestor;
        if ((p_info_list->p_base + ancestor_with_min_semi_id)->min_semi_id < (p_info_list->p_base + node)->min_semi_id)
            (p_info_list->p_base + node)->min_semi_id = (p_info_list->p_base + ancestor_with_min_semi_id)->min_semi_id;
    }
    return (p_info_list->p_base + node)->min_semi_id;
}
static inline void set_dom_depth(p_ir_basic_block p_basic_block, size_t depth) {
    p_basic_block->dom_depth = depth;
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->dom_son_list) {
        p_ir_basic_block p_son = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        set_dom_depth(p_son, depth + 1);
    }
}
static inline void clear_dom_info(p_ir_basic_block p_basic_block) {
    p_list_head p_node, p_node_next;
    list_for_each_safe(p_node, p_node_next, &p_basic_block->dom_son_list) {
        p_ir_basic_block_list_node p_son = list_entry(p_node, ir_basic_block_list_node, node);
        clear_dom_info(p_son->p_basic_block);
        ir_basic_block_list_node_drop(p_son);
    }
    p_basic_block->p_dom_parent = NULL;
}
// 必须保证图连通， 否则会出错
#include <stdio.h>
void ir_cfg_set_func_dom(p_symbol_func p_func) {
    if (list_head_alone(&p_func->block)) return;
    printf("--- build dom tree %s ---\n", p_func->name);
    if (!p_func->if_updated_graph) {
        printf("    control graph not changed!\n");
        return;
    }
    clear_dom_info(p_func->p_entry_block);
    size_t block_num = p_func->block_cnt;
    p_cfg_build_dom_tree_info_list p_info_list = malloc(sizeof(*p_info_list));
    p_info_list->block_num = block_num;
    p_info_list->block2dfn_id = malloc(sizeof(*p_info_list->block2dfn_id) * block_num);
    p_info_list->p_base = malloc(sizeof(*p_info_list->p_base) * block_num);
    // 初始化 dfs 序
    symbol_func_basic_block_init_visited(p_func);
    p_ir_basic_block p_entry = list_entry(p_func->block.p_next, ir_basic_block, node);
    init_dfs_sequence(p_info_list, 0, 0, p_entry);

    for (size_t i = block_num - 1; i >= 1; i--) {
        p_cfg_build_dom_tree_info p_info = p_info_list->p_base + i;
        p_list_head p_node;
        size_t min_semi_id = p_info->parent;
        // 计算半支配点
        list_for_each(p_node, &p_info->p_basic_block->prev_basic_block_list) {
            p_ir_basic_block p_prev = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            if (p_info_list->block2dfn_id[p_prev->block_id] <= i) // 若是真祖先取最小的
                min_semi_id = min_semi_id > p_info_list->block2dfn_id[p_prev->block_id] ? p_info_list->block2dfn_id[p_prev->block_id] : min_semi_id;
            else {
                size_t prev_with_min_semi_id = ancestor_with_lowest_semi(p_info_list, p_info_list->block2dfn_id[p_prev->block_id]);
                size_t prev_min_semi_id = (p_info_list->p_base + prev_with_min_semi_id)->semi_block;
                min_semi_id = min_semi_id > prev_min_semi_id ? prev_min_semi_id : min_semi_id;
            }
        }
        p_info->semi_block = min_semi_id;
        // 现在这个点到半支配点的路径上的点的半支配点都不明确，需要暂存直到将此半支配点加入树中再计算支配点
        (p_info_list->p_base + min_semi_id)->p_semi_block_list[(p_info_list->p_base + min_semi_id)->current_num_semi++] = i;
        // 将父节点加入当前树中， 并求所有以父节点为半支配点的节点的支配点, 此时从父节点到半支配儿子的路径上的点的半支配点都已确定
        p_info->ancestor = p_info->parent; // 既是节点 i 生成树上的父亲 又是半支配树上的父亲
        p_info->min_semi_id = i;
        p_cfg_build_dom_tree_info p_parent_info = p_info_list->p_base + p_info->parent;
        for (size_t j = 0; j < p_parent_info->current_num_semi; j++) {
            size_t semi_son = p_parent_info->p_semi_block_list[j];
            // 计算从半支配树上的父节点到子节点对应的 dfs 树上的路径所有节点的半支配点的最小值

            min_semi_id = ancestor_with_lowest_semi(p_info_list, semi_son);
            if ((p_info_list->p_base + min_semi_id)->semi_block == p_info->parent) // 半支配点与支配点重合
                ir_basic_block_add_dom_son(p_parent_info->p_basic_block, (p_info_list->p_base + semi_son)->p_basic_block);
            //(p_info_list->p_base + semi_son)->p_basic_block->p_dom_parent = p_parent_info->p_basic_block;
            else // 否则直接支配点为最小的半支配点节点对应的支配点， 此时支配点可能未知， 需要暂存这个点，之后计算
                (p_info_list->p_base + semi_son)->p_basic_block->p_dom_parent = (p_info_list->p_base + min_semi_id)->p_basic_block;
        }
        // 置空 半支配子节点， 否则会影响之后的生成
        p_parent_info->current_num_semi = 0;
    }

    // 回填支配点
    for (size_t i = 1; i < block_num; i++) {
        p_cfg_build_dom_tree_info p_info = p_info_list->p_base + i;
        // 若该节点的半支配点与支配点相同， 说明之前已经计算出了支配点（第二种情况假定的支配点必定在半支配点之下）
        if (p_info->p_basic_block->p_dom_parent != (p_info_list->p_base + p_info->semi_block)->p_basic_block)
            ir_basic_block_add_dom_son(p_info->p_basic_block->p_dom_parent->p_dom_parent, p_info->p_basic_block);
    }
    set_dom_depth(p_func->p_entry_block, 0);
    cfg_build_dom_tree_info_list_drop(p_info_list);
    p_func->if_updated_graph = false;
    ir_basic_block_dom_info_print(p_func->p_entry_block);
}

bool ir_basic_block_dom_check(p_ir_basic_block p_son, p_ir_basic_block p_parent) {
    while (p_son->dom_depth > p_parent->dom_depth)
        p_son = p_son->p_dom_parent;
    if (p_son == p_parent)
        return true;
    return false;
}

void ir_cfg_set_program_dom(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        ir_cfg_set_func_dom(p_func);
    }
}
