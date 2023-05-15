#include <mir/bb_param.h>
#include <mir_print.h>

#include <stdio.h>
void mir_bb_phi_list_print(p_mir_bb_phi_list p_bb_phi_list) {
    assert(p_bb_phi_list);
    p_list_head p_node;
    printf("(");
    list_for_each(p_node, &p_bb_phi_list->bb_phi) {
        p_mir_bb_phi p_bb_phi = list_entry(p_node, mir_bb_phi, node);
        mir_vreg_print(p_bb_phi->p_bb_phi);
        if (p_node->p_next != &p_bb_phi_list->bb_phi)
            printf(", ");
    }
    printf(")");
}
void mir_bb_param_list_print(p_mir_bb_param_list p_bb_param_list) {
    assert(p_bb_param_list);
    p_list_head p_node;
    printf("(");
    list_for_each(p_node, &p_bb_param_list->bb_param) {
        p_mir_bb_param p_bb_param = list_entry(p_node, mir_bb_param, node);
        if (!p_bb_param->p_bb_param) {
            printf("-");
        }
        else {
            mir_operand_print(p_bb_param->p_bb_param);
        }
        if (p_node->p_next != &p_bb_param_list->bb_param)
            printf(", ");
    }
    printf(")");
}