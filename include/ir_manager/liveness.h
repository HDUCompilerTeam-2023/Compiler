#ifndef __REG_ALLOC_GRAPH_ALLOC_LIVENESS__
#define __REG_ALLOC_GRAPH_ALLOC_LIVENESS__
#include <ir.h>
typedef uint32_t *vreg_uint_map;

typedef struct liveness_info liveness_info, *p_liveness_info;

struct liveness_info{
    p_symbol_func p_func;
    vreg_uint_map *block_live_in;
    vreg_uint_map *block_live_out;
    vreg_uint_map *block_branch_live_in;
    vreg_uint_map *instr_live_in;
    vreg_uint_map *instr_live_out;
    size_t *edge_length;
    size_t instr_num;
    p_ir_vreg *p_vregs;
    size_t vreg_num;
    p_ir_vreg_list *graph_list;
    bool **graph_table;
};

void liveness_analysis(p_liveness_info p_info);
p_liveness_info liveness_info_gen(p_symbol_func p_func);
void liveness_info_drop(p_liveness_info p_info);

#endif
