#ifndef __IR_MANAGER_BUILDCDG__
#define __IR_MANAGER_BUILDCDG__
#include <ir.h>
#include <program/use.h>
typedef struct reverse_dom_tree_info reverse_dom_tree_info, *p_reverse_dom_tree_info;
typedef struct reverse_dom_tree_info_list reverse_dom_tree_info_list, *p_reverse_dom_tree_info_list;
struct reverse_dom_tree_info {
    p_bitmap reverse_dom_frontier; // 支配边界

    p_ir_basic_block p_basic_block; // 基本块
    p_ir_basic_block reverse_dom_parent;
    p_ir_basic_block_list reverse_dom_son_list;

    p_ir_basic_block_list p_cdg_prev; // 控制依赖图上的前驱
    p_ir_basic_block_list p_cdg_next; // 后继

    size_t semi_block; // 该基本块的半支配节点
    struct {
        size_t *p_semi_block_list; // 所有以该基本块为半支配节点的节点
        size_t current_num_semi;
    };

    size_t parent; // 生成树上的父节点

    // 带权并查集的祖先和到祖先的路径上的具有最小半支配节点编号的节点的半支配节点编号
    size_t ancestor;
    size_t min_semi_id;
};

struct reverse_dom_tree_info_list {
    p_reverse_dom_tree_info p_base;
    size_t block_num;
    size_t *block2dfn_id;
};

void reverse_dom_tree_info_list_drop(p_reverse_dom_tree_info_list p_info_list);
p_reverse_dom_tree_info_list ir_build_cdg_func(p_symbol_func p_func);
void ir_build_cdg_pass(p_program p_ir);
#endif