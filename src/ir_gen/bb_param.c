#include <ir_gen.h>
#include <ir_gen/bb_param.h>

p_ir_bb_phi_list ir_bb_phi_list_init(void) {
    p_ir_bb_phi_list p_bb_phi_list = malloc(sizeof(*p_bb_phi_list));
    *p_bb_phi_list = (ir_bb_phi_list) {
        .bb_phi = list_head_init(&p_bb_phi_list->bb_phi)
    };
    return p_bb_phi_list;
}
p_ir_bb_phi_list ir_bb_phi_list_add(p_ir_bb_phi_list p_bb_phi_list, p_ir_vreg p_bb_phi) {
    p_ir_bb_phi p_ir_bb_phi = malloc(sizeof(*p_ir_bb_phi));
    *p_ir_bb_phi = (ir_bb_phi) {
        .p_bb_phi = p_bb_phi,
        .node = list_head_init(&p_ir_bb_phi->node),
    };

    list_add_prev(&p_ir_bb_phi->node, &p_bb_phi_list->bb_phi);
    return p_bb_phi_list;
}
void ir_bb_phi_list_drop(p_ir_bb_phi_list p_bb_phi_list) {
    assert(p_bb_phi_list);
    while (!list_head_alone(&p_bb_phi_list->bb_phi)) {
        p_ir_bb_phi p_bb_phi = list_entry(p_bb_phi_list->bb_phi.p_next, ir_bb_phi, node);
        list_del(&p_bb_phi->node);
        free(p_bb_phi);
    }
    free(p_bb_phi_list);
}

void copy_live(p_ir_bb_phi_list p_des, p_ir_bb_phi_list p_src) {
    p_list_head p_node;
    list_for_each(p_node, &p_src->bb_phi) {
        p_ir_vreg p_param = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        ir_bb_phi_list_add(p_des, p_param);
    }
}

bool if_in_live_set(p_ir_bb_phi_list p_list, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_list->bb_phi) {
        p_ir_vreg p_live = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        if (p_live == p_vreg)
            return true;
    }
    return false;
}

void live_set_del(p_ir_bb_phi_list p_list, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_list->bb_phi) {
        p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
        if (p_phi->p_bb_phi == p_vreg) {
            list_del(&p_phi->node);
            free(p_phi);
            break;
        }
    }
}
