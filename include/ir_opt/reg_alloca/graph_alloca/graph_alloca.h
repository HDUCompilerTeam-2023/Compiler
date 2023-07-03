#ifndef __IR_OPT_REG_ALLOC_GRAPH_ALLOC__
#define __IR_OPT_REG_ALLOC_GRAPH_ALLOC__
#include <ir_opt/reg_alloca/graph_alloca/conflict_graph.h>
typedef struct graph_alloca_info graph_alloca_info, *p_graph_alloca_info;

struct graph_alloca_info {
    p_symbol_func p_func;
    p_conflict_graph p_graph;
    p_bitmap *block_live_in;
    p_bitmap *block_live_out;
    p_bitmap *block_branch_live_in;
    p_bitmap *instr_live_in;
    p_bitmap *instr_live_out;
    size_t instr_num;
    p_ir_vreg *p_vregs;
};

void graph_alloca(p_symbol_func p_func, size_t reg_num_r, size_t reg_num_s);

#endif