#ifndef __MIR_MANGER_BUILDDOMTREE__
#define __MIR_MANGER_BUILDDOMTREE__

#include <mir.h>
#include <program/use.h>

typedef struct cfg_build_dom_tree_info cfg_build_dom_tree_info, *p_cfg_build_dom_tree_info;

struct cfg_build_dom_tree_info {
    p_mir_basic_block p_basic_block; // 基本块
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

size_t init_dfs_sequence(cfg_build_dom_tree_info dfs_seq[], size_t block_num, size_t current_num, size_t parent, p_mir_basic_block p_entry);
void cfg_build_dom_tree_info_drop(p_cfg_build_dom_tree_info p_dom_info);
void cfg_build_dom_tree_info_gen(cfg_build_dom_tree_info dom_info[], p_mir_basic_block p_basic_block, size_t block_num, size_t parent, size_t current_num);
void mir_cfg_set_program_dom(p_program p_program);
void mir_cfg_set_func_dom(p_symbol_func p_func);

#endif
