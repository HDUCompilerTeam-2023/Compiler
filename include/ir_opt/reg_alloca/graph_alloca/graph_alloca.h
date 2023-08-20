#ifndef __IR_OPT_REG_ALLOC_GRAPH_ALLOC__
#define __IR_OPT_REG_ALLOC_GRAPH_ALLOC__
#include <ir_opt/reg_alloca/graph_alloca/conflict_graph.h>
#include <ir.h>

p_conflict_graph ir_gen_conflict_graph(p_symbol_func p_func, size_t reg_num_r, size_t reg_num_s);
void ir_graph_spill(p_symbol_func p_func, p_conflict_graph p_graph);
void graph_alloca(p_symbol_func p_func, p_conflict_graph p_graph);
void ir_combine(p_symbol_func p_func, p_conflict_graph p_graph);

#endif