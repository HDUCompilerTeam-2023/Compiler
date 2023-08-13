#ifndef __IR_MANAGER_GET_SEQS__
#define __IR_MANAGER_GET_SEQS__
#include <symbol/func.h>
p_list_head get_topo_seqs(p_symbol_func p_func);
p_list_head get_rpo_seqs(p_symbol_func p_func);

#endif