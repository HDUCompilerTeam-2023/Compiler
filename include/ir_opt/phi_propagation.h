#ifndef __IR_OPT_PHI_PROPAGATION__
#define __IR_OPT_PHI_PROPAGATION__

#include <program/use.h>
#include <symbol/func.h>

void ir_func_phi_propagation(p_symbol_func p_func);
void ir_phi_propagation(p_program p_ir);

#endif
