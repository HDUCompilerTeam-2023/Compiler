#ifndef __REG_ALLOC_GRAPH_ALLOC_LIVENESS__
#define __REG_ALLOC_GRAPH_ALLOC_LIVENESS__
#include <ir.h>
typedef struct liveness_info liveness_info, *p_liveness_info;

struct liveness_info{
    p_symbol_func p_func;
    p_bitmap *block_live_in;
    p_bitmap *block_live_out;
    p_bitmap *block_branch_live_in;
    p_bitmap *instr_live_in;
    p_bitmap *instr_live_out;
    size_t instr_num;
    p_ir_vreg *p_vregs;
    size_t vreg_num;
    bool **graph_table;
    bool use_table;
};

void liveness_analysis(p_liveness_info p_info);
p_liveness_info liveness_info_gen(p_symbol_func p_func);
void liveness_info_drop(p_liveness_info p_info);

#endif
