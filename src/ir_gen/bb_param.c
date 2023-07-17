#include <ir_gen.h>
#include <ir_gen/bb_param.h>

void ir_bb_phi_set_vreg(p_ir_bb_phi p_bb_phi, p_ir_vreg p_vreg) {
    p_bb_phi->p_bb_phi = p_vreg;
    p_vreg->p_bb_phi = p_bb_phi;
}

