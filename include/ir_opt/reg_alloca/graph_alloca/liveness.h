#ifndef __REG_ALLOC_GRAPH_ALLOC_LIVENESS__
#define __REG_ALLOC_GRAPH_ALLOC_LIVENESS__
#include <ir_opt/reg_alloca/graph_alloca/graph_alloca.h>

void check_liveness(p_symbol_func p_func);
void set_func_live(p_graph_alloca_info p_info, p_symbol_func p_func);
void liveness_analysis(p_graph_alloca_info p_info, p_symbol_func p_func);

#endif