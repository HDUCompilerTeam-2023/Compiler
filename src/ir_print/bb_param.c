#include <ir/bb_param.h>
#include <ir_print.h>

#include <stdio.h>
void ir_bb_phi_print(p_ir_bb_phi p_phi) {
    ir_vreg_print(p_phi->p_bb_phi);
}
void ir_varray_bb_phi_print(p_ir_varray_bb_phi p_varray_phi) {
    ir_varray_print(p_varray_phi->p_varray_phi);
}
void ir_bb_param_print(p_ir_bb_param p_bb_param) {
    if (!p_bb_param->p_bb_param) {
        printf("-");
    }
    else {
        ir_operand_print(p_bb_param->p_bb_param);
    }
}
void ir_varray_bb_param_print(p_ir_varray_bb_param p_varray_bb_param) {
    if (!p_varray_bb_param->p_varray_bb_param) {
        printf("-");
    }
    else {
        ir_varray_use_print(p_varray_bb_param->p_varray_bb_param);
    }
}