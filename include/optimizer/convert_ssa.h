#ifndef __OPTIMIZER__CONVERT_SSA__
#define __OPTIMIZER__CONVERT_SSA__

#include <mir_gen.h>

struct convert_ssa{
    p_mir_basic_block p_basic_block; 
    p_bitmap dom_frontier; // 支配边界
};
typedef struct convert_ssa convert_ssa, *p_convert_ssa;

void convert_ssa_gen(convert_ssa *dfs_seq, size_t block_num, p_mir_basic_block p_basic_block, size_t current_num);
size_t convert_ssa_init_dfs_sequence(convert_ssa *dfs_seq, size_t block_num, p_mir_basic_block p_entry, size_t current_num);
void convert_ssa_compute_dom_frontier(convert_ssa *dfs_seq, size_t block_num);

void convert_ssa_func(p_mir_func p_func);
void convert_ssa_program(p_mir_program p_program);

void convert_ssa_dfs_seq_drop(convert_ssa *dfs_seq, size_t block_num);
#endif