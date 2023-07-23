#include <ir_gen.h>
#include <ir_manager/buildcdg.h>
#include <program/def.h>
#include <stdio.h>
#include <symbol_gen.h>

static inline void reverse_dom_tree_info_gen(p_reverse_dom_tree_info_list p_info_list, p_ir_basic_block p_basic_block, size_t parent, size_t current_num) {
    *(p_info_list->p_base + current_num) = (reverse_dom_tree_info) {
        .p_basic_block = p_basic_block,
        .p_semi_block_list = malloc(sizeof(size_t) * (p_info_list->block_num - current_num)),
        .current_num_semi = 0,
        .parent = parent,
        .ancestor = -1,
        .reverse_dom_parent = NULL,
        .reverse_dom_son_list = ir_basic_block_list_init(),
        .reverse_dom_frontier = bitmap_gen(p_info_list->block_num),
        .p_cdg_next = ir_basic_block_list_init(),
        .p_cdg_prev = ir_basic_block_list_init(),
    };
    bitmap_set_empty((p_info_list->p_base + current_num)->reverse_dom_frontier);
    p_info_list->block2dfn_id[p_basic_block->block_id] = current_num;
}
void reverse_dom_tree_info_list_drop(p_reverse_dom_tree_info_list p_info_list) {
    for (size_t i = 0; i < p_info_list->block_num; i++) {
        free((p_info_list->p_base + i)->p_semi_block_list);
        ir_basic_block_list_drop((p_info_list->p_base + i)->reverse_dom_son_list);
        ir_basic_block_list_drop((p_info_list->p_base + i)->p_cdg_next);
        ir_basic_block_list_drop((p_info_list->p_base + i)->p_cdg_prev);
        bitmap_drop((p_info_list->p_base + i)->reverse_dom_frontier);
    }
    free(p_info_list->p_base);
    free(p_info_list->block2dfn_id);
    free(p_info_list);
}
static inline size_t init_reverse_dfs_sequence(p_reverse_dom_tree_info_list p_info_list, size_t current_num, size_t parent, p_ir_basic_block p_exit) {
    if (p_exit->if_visited) return current_num;
    p_exit->if_visited = true;
    size_t dfn_id = current_num;
    reverse_dom_tree_info_gen(p_info_list, p_exit, parent, dfn_id);
    current_num++;

    p_list_head p_node;
    list_for_each(p_node, &p_exit->prev_branch_target_list) {
        p_ir_basic_block p_prev = list_entry(p_node, ir_branch_target_node, node)->p_target->p_source_block;
        current_num = init_reverse_dfs_sequence(p_info_list, current_num, dfn_id, p_prev);
    }
    return current_num;
}

// 带权并查集的查找
static inline size_t ancestor_with_lowest_semi(p_reverse_dom_tree_info_list p_info_list, size_t node) {
    size_t a = (p_info_list->p_base + node)->ancestor;
    if ((p_info_list->p_base + a)->ancestor != -1) { // 不能找祖先的 ancestor
        size_t ancestor_with_min_semi_id = ancestor_with_lowest_semi(p_info_list, a);
        (p_info_list->p_base + node)->ancestor = (p_info_list->p_base + a)->ancestor;
        if ((p_info_list->p_base + ancestor_with_min_semi_id)->min_semi_id < (p_info_list->p_base + node)->min_semi_id)
            (p_info_list->p_base + node)->min_semi_id = (p_info_list->p_base + ancestor_with_min_semi_id)->min_semi_id;
    }
    return (p_info_list->p_base + node)->min_semi_id;
}

static inline void ir_reverse_dom_tree_print(p_reverse_dom_tree_info_list p_list, p_ir_basic_block p_exit, size_t depth) {
    for (size_t i = 0; i < depth << 2; i++)
        printf("-");
    printf("b%ld (depth:%ld)\n", p_exit->block_id, depth);
    p_reverse_dom_tree_info p_info = p_list->p_base + p_list->block2dfn_id[p_exit->block_id];
    p_list_head p_node;
    list_for_each(p_node, &p_info->reverse_dom_son_list->block_list) {
        p_ir_basic_block p_son = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        ir_reverse_dom_tree_print(p_list, p_son, depth + 1);
    }
}
static inline void ir_build_reverse_dom_tree(p_reverse_dom_tree_info_list p_info_list) {
    for (size_t i = p_info_list->block_num - 1; i >= 1; i--) {
        p_reverse_dom_tree_info p_info = p_info_list->p_base + i;
        size_t min_semi_id = p_info->parent;
        // 计算半支配点
        p_ir_basic_block_branch_target p_target_true = p_info->p_basic_block->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_target_false = p_info->p_basic_block->p_branch->p_target_2;
        if (p_target_true) {
            p_ir_basic_block p_next = p_target_true->p_block;
            if (p_info_list->block2dfn_id[p_next->block_id] <= i) // 若是真祖先取最小的
                min_semi_id = min_semi_id > p_info_list->block2dfn_id[p_next->block_id] ? p_info_list->block2dfn_id[p_next->block_id] : min_semi_id;
            else {
                size_t prev_with_min_semi_id = ancestor_with_lowest_semi(p_info_list, p_info_list->block2dfn_id[p_next->block_id]);
                size_t prev_min_semi_id = (p_info_list->p_base + prev_with_min_semi_id)->semi_block;
                min_semi_id = min_semi_id > prev_min_semi_id ? prev_min_semi_id : min_semi_id;
            }
        }
        if (p_target_false) {
            p_ir_basic_block p_next = p_target_false->p_block;
            if (p_info_list->block2dfn_id[p_next->block_id] <= i) // 若是真祖先取最小的
                min_semi_id = min_semi_id > p_info_list->block2dfn_id[p_next->block_id] ? p_info_list->block2dfn_id[p_next->block_id] : min_semi_id;
            else {
                size_t prev_with_min_semi_id = ancestor_with_lowest_semi(p_info_list, p_info_list->block2dfn_id[p_next->block_id]);
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
        p_reverse_dom_tree_info p_parent_info = p_info_list->p_base + p_info->parent;
        for (size_t j = 0; j < p_parent_info->current_num_semi; j++) {
            size_t semi_son = p_parent_info->p_semi_block_list[j];
            // 计算从半支配树上的父节点到子节点对应的 dfs 树上的路径所有节点的半支配点的最小值

            min_semi_id = ancestor_with_lowest_semi(p_info_list, semi_son);
            if ((p_info_list->p_base + min_semi_id)->semi_block == p_info->parent) // 半支配点与支配点重合
            {
                ir_basic_block_list_add(p_parent_info->reverse_dom_son_list, (p_info_list->p_base + semi_son)->p_basic_block);
                (p_info_list->p_base + semi_son)->reverse_dom_parent = p_parent_info->p_basic_block;
            }
            //(p_info_list->p_base + semi_son)->p_basic_block->p_dom_parent = p_parent_info->p_basic_block;
            else // 否则直接支配点为最小的半支配点节点对应的支配点， 此时支配点可能未知， 需要暂存这个点，之后计算
                (p_info_list->p_base + semi_son)->reverse_dom_parent = (p_info_list->p_base + min_semi_id)->p_basic_block;
        }
        // 置空 半支配子节点， 否则会影响之后的生成
        p_parent_info->current_num_semi = 0;
    }

    // 回填支配点
    for (size_t i = 1; i < p_info_list->block_num; i++) {
        p_reverse_dom_tree_info p_info = p_info_list->p_base + i;
        // 若该节点的半支配点与支配点相同， 说明之前已经计算出了支配点（第二种情况假定的支配点必定在半支配点之下）
        if (p_info->reverse_dom_parent != (p_info_list->p_base + p_info->semi_block)->p_basic_block) {
            p_reverse_dom_tree_info p_tmp_info = p_info_list->p_base + p_info_list->block2dfn_id[p_info->reverse_dom_parent->block_id];
            p_reverse_dom_tree_info p_dom_parent_info = p_info_list->p_base + p_info_list->block2dfn_id[p_tmp_info->reverse_dom_parent->block_id];
            ir_basic_block_list_add(p_dom_parent_info->reverse_dom_son_list, p_info->p_basic_block);
            p_info->reverse_dom_parent = p_dom_parent_info->p_basic_block;
        }
    }
}

static inline void ir_compute_reverse_dom_frontier(p_reverse_dom_tree_info_list p_info_list) {
    for (size_t i = p_info_list->block_num - 1; i < p_info_list->block_num; i--) {
        p_reverse_dom_tree_info p_info = p_info_list->p_base + i;
        p_list_head p_node;
        list_for_each(p_node, &p_info->p_basic_block->prev_branch_target_list) {
            p_ir_basic_block p_prev = list_entry(p_node, ir_branch_target_node, node)->p_target->p_source_block;
            bitmap_add_element(p_info->reverse_dom_frontier, p_prev->block_id);
        }

        // 记录 直接支配点
        p_bitmap p_son_list = bitmap_gen(p_info_list->block_num);
        bitmap_set_empty(p_son_list);
        // 将支配树上的直接儿子的支配边界作为候选
        list_for_each(p_node, &p_info->reverse_dom_son_list->block_list) {
            p_ir_basic_block p_son = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            p_reverse_dom_tree_info p_son_info = p_info_list->p_base + p_info_list->block2dfn_id[p_son->block_id];
            bitmap_add_element(p_son_list, p_son->block_id);
            bitmap_merge_not_new(p_info->reverse_dom_frontier, p_son_info->reverse_dom_frontier);
        }
        // 所有候选中不受当前节点直接支配的节点为支配边界
        bitmap_neg_not_new(p_son_list);
        bitmap_and_not_new(p_info->reverse_dom_frontier, p_son_list);
        bitmap_drop(p_son_list);
    }
}
static inline void ir_reverse_dom_frontier_cdg(p_reverse_dom_tree_info_list p_info_list) {
    for (size_t i = 0; i < p_info_list->block_num; i++) {
        p_reverse_dom_tree_info p_info = p_info_list->p_base + i;
        for (size_t j = 0; j < p_info_list->block_num; j++) {
            if (bitmap_if_in(p_info->reverse_dom_frontier, j)) {
                size_t dfn_id = p_info_list->block2dfn_id[j];
                ir_basic_block_list_add(p_info->p_cdg_prev, (p_info_list->p_base + dfn_id)->p_basic_block);
                ir_basic_block_list_add((p_info_list->p_base + dfn_id)->p_cdg_next, p_info->p_basic_block);
            }
        }
    }
}

static inline void ir_basic_block_cdg_print(p_reverse_dom_tree_info p_info) {
    printf("b%ld:\n", p_info->p_basic_block->block_id);
    p_list_head p_node;
    printf("    prev: {");
    list_for_each(p_node, &p_info->p_cdg_prev->block_list) {
        p_ir_basic_block p_prev = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        printf("b%ld", p_prev->block_id);
        if (p_node->p_next != &p_info->p_cdg_prev->block_list)
            printf(", ");
    }
    printf("}\n");

    printf("    next: {");
    list_for_each(p_node, &p_info->p_cdg_next->block_list) {
        p_ir_basic_block p_next = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        printf("b%ld", p_next->block_id);
        if (p_node != p_info->p_cdg_next->block_list.p_prev)
            printf(", ");
    }
    printf("}\n");
}
static inline void ir_func_cdg_print(p_reverse_dom_tree_info_list p_info_list) {
    for (size_t i = 0; i < p_info_list->block_num; i++) {
        ir_basic_block_cdg_print(p_info_list->p_base + i);
    }
}

p_reverse_dom_tree_info_list ir_build_cdg_func(p_symbol_func p_func) {
    if (list_head_alone(&p_func->block)) return NULL;
    // printf("build cdg :\n");
    symbol_func_set_block_id(p_func);
    size_t block_num = p_func->block_cnt;
    p_reverse_dom_tree_info_list p_info_list = malloc(sizeof(*p_info_list));
    p_info_list->block_num = block_num;
    p_info_list->block2dfn_id = malloc(sizeof(*p_info_list->block2dfn_id) * block_num);
    p_info_list->p_base = malloc(sizeof(*p_info_list->p_base) * block_num);
    // 初始化 dfs 序
    symbol_func_basic_block_init_visited(p_func);
    init_reverse_dfs_sequence(p_info_list, 0, 0, p_func->p_ret_block);

    ir_build_reverse_dom_tree(p_info_list);
    if (0)
        ir_reverse_dom_tree_print(p_info_list, p_func->p_ret_block, 0);
    ir_compute_reverse_dom_frontier(p_info_list);
    ir_reverse_dom_frontier_cdg(p_info_list);
    if (0)
        ir_func_cdg_print(p_info_list);
    return p_info_list;
}
void ir_build_cdg_pass(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        ir_build_cdg_func(p_func);
    }
}