#include <ir/bb_param.h>
#include <ir_print.h>

#include <stdio.h>
void ir_bb_phi_list_print(p_ir_bb_phi_list p_bb_phi_list) {
    assert(p_bb_phi_list);
    p_list_head p_node;
    printf("(");
    list_for_each(p_node, &p_bb_phi_list->bb_phi) {
        p_ir_bb_phi p_bb_phi = list_entry(p_node, ir_bb_phi, node);
        ir_vreg_print(p_bb_phi->p_bb_phi);
        if (p_node->p_next != &p_bb_phi_list->bb_phi)
            printf(", ");
    }
    printf(")");
}

void ir_bb_param_print(p_ir_bb_param p_bb_param) {
    if (!p_bb_param->p_bb_param) {
        printf("-");
    }
    else {
        ir_operand_print(p_bb_param->p_bb_param);
    }
}
