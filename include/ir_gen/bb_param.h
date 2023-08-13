#ifndef __IR_GEN_BB_PARAM__
#define __IR_GEN_BB_PARAM__
#include <ir/bb_param.h>
void ir_bb_phi_set_vreg(p_ir_bb_phi p_bb_phi, p_ir_vreg p_vreg);
void ir_varray_bb_phi_set_varry(p_ir_varray_bb_phi p_varray_bb_phi, p_ir_varray p_varray);
void ir_varray_bb_phi_drop(p_ir_varray_bb_phi p_varray_bb_phi);
void ir_varray_bb_param_drop(p_ir_varray_bb_param p_varray_param);
#endif
