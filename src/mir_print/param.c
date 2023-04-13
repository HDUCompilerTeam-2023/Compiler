#include <mir_print.h>
#include <mir/param.h>

#include <stdio.h>
void mir_param_list_print(p_mir_param_list p_param_list)
{
    assert(p_param_list);
    p_list_head p_node;
    printf("(");
    list_for_each(p_node, &p_param_list->param){
        p_mir_param p_param = list_entry(p_node, mir_param, node);
        mir_operand_print(p_param->p_param);
        if (p_node->p_next != &p_param_list->param)
            printf(", ");
    }
    printf(")");
}