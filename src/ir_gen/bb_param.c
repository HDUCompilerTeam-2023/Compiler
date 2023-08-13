#include <ir_gen.h>
#include <ir_gen/bb_param.h>

void ir_bb_phi_set_vreg(p_ir_bb_phi p_bb_phi, p_ir_vreg p_vreg) {
    p_bb_phi->p_bb_phi = p_vreg;
    p_vreg->p_bb_phi = p_bb_phi;
}
void ir_varray_bb_phi_set_varry(p_ir_varray_bb_phi p_varray_bb_phi, p_ir_varray p_varray) {
    p_varray_bb_phi->p_varray_phi = p_varray;
    p_varray->p_varray_bb_phi = p_varray_bb_phi;
}
void ir_varray_bb_phi_drop(p_ir_varray_bb_phi p_varray_bb_phi) {
    assert(p_varray_bb_phi->p_varray_phi->varray_def_type == varray_bb_phi_def);
    assert(p_varray_bb_phi->p_varray_phi->p_varray_bb_phi == p_varray_bb_phi);
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_varray_bb_phi->varray_param_list) {
        p_ir_varray_bb_param p_param = list_entry(p_node, ir_varray_bb_param, phi_node);
        assert(p_param->p_des_phi == p_varray_bb_phi);
        ir_varray_bb_param_drop(p_param);
    }
    list_del(&p_varray_bb_phi->node);
    free(p_varray_bb_phi);
}
void ir_varray_bb_param_drop(p_ir_varray_bb_param p_varray_param) {
    assert(p_varray_param->p_des_phi->p_basic_block == p_varray_param->p_target->p_block);
    if (p_varray_param->p_varray_bb_param) {
        assert(p_varray_param->p_varray_bb_param->varray_use_type == varray_bb_param_use);
        assert(p_varray_param->p_varray_bb_param->p_varray_param == p_varray_param);
        ir_varray_use_drop(p_varray_param->p_varray_bb_param);
    }
    list_del(&p_varray_param->phi_node);
    list_del(&p_varray_param->node);
    free(p_varray_param);
}