#include <mir_gen.h>
#include <mir_gen/bb_param.h>

p_mir_bb_phi_list mir_bb_phi_list_init(void) {
    p_mir_bb_phi_list p_bb_phi_list = malloc(sizeof(*p_bb_phi_list));
    *p_bb_phi_list = (mir_bb_phi_list) {
        .bb_phi = list_head_init(&p_bb_phi_list->bb_phi)
    };
    return p_bb_phi_list;
}
p_mir_bb_phi_list mir_bb_phi_list_add(p_mir_bb_phi_list p_bb_phi_list, p_mir_vreg p_bb_phi) {
    p_mir_bb_phi p_ir_bb_phi = malloc(sizeof(*p_ir_bb_phi));
    *p_ir_bb_phi = (mir_bb_phi) {
        .p_bb_phi = p_bb_phi,
        .node = list_head_init(&p_ir_bb_phi->node),
    };

    list_add_prev(&p_ir_bb_phi->node, &p_bb_phi_list->bb_phi);
    return p_bb_phi_list;
}
void mir_bb_phi_list_drop(p_mir_bb_phi_list p_bb_phi_list) {
    assert(p_bb_phi_list);
    while (!list_head_alone(&p_bb_phi_list->bb_phi)) {
        p_mir_bb_phi p_bb_phi = list_entry(p_bb_phi_list->bb_phi.p_next, mir_bb_phi, node);
        list_del(&p_bb_phi->node);
        free(p_bb_phi);
    }
    free(p_bb_phi_list);
}

p_mir_bb_param_list mir_bb_param_list_init(void) {
    p_mir_bb_param_list p_bb_param_list = malloc(sizeof(*p_bb_param_list));
    *p_bb_param_list = (mir_bb_param_list) {
        .bb_param = list_head_init(&p_bb_param_list->bb_param)
    };
    return p_bb_param_list;
}
p_mir_bb_param_list mir_bb_param_list_add(p_mir_bb_param_list p_bb_param_list, p_mir_operand p_bb_param) {
    p_mir_bb_param p_ir_bb_param = malloc(sizeof(*p_ir_bb_param));
    *p_ir_bb_param = (mir_bb_param) {
        .p_bb_param = p_bb_param,
        .node = list_head_init(&p_ir_bb_param->node),
    };

    list_add_prev(&p_ir_bb_param->node, &p_bb_param_list->bb_param);
    return p_bb_param_list;
}
void mir_bb_param_list_drop(p_mir_bb_param_list p_bb_param_list) {
    assert(p_bb_param_list);
    while (!list_head_alone(&p_bb_param_list->bb_param)) {
        p_mir_bb_param p_bb_param = list_entry(p_bb_param_list->bb_param.p_next, mir_bb_param, node);
        if (p_bb_param->p_bb_param)
            mir_operand_drop(p_bb_param->p_bb_param);
        list_del(&p_bb_param->node);
        free(p_bb_param);
    }
    free(p_bb_param_list);
}
