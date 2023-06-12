#include <ir/param.h>
#include <ir_print.h>

#include <stdio.h>
void ir_param_list_print(p_ir_param_list p_param_list) {
    assert(p_param_list);
    p_list_head p_node;
    printf("(");
    list_for_each(p_node, &p_param_list->param) {
        p_ir_param p_param = list_entry(p_node, ir_param, node);
        ir_operand_print(p_param->p_param);
        if (p_node->p_next != &p_param_list->param)
            printf(", ");
    }
    printf(")");
}
