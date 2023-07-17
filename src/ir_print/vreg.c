#include <ir/vreg.h>
#include <ir_print.h>
#include <symbol_print.h>

#include <stdio.h>

void ir_vreg_print(p_ir_vreg p_vreg) {
    assert(p_vreg);
    symbol_type_print(p_vreg->p_type);
    printf(" %%%ld", p_vreg->id);
    if (p_vreg->reg_id != -1)
        printf(" (reg_id: %ld, ", p_vreg->reg_id);
    else {
        printf("(");
    }
    printf("if_float: %d)", p_vreg->if_float);
}

void ir_vreg_list_print(p_ir_vreg_list p_vreg_list) {
    assert(p_vreg_list);
    p_list_head p_node;
    printf("(");
    list_for_each(p_node, &p_vreg_list->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg_list_node, node)->p_vreg;
        ir_vreg_print(p_vreg);
        if (p_node->p_next != &p_vreg_list->vreg_list)
            printf(", ");
    }
    printf(")");
}