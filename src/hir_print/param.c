#include <hir_print.h>
#include <stdio.h>

#include <hir/param.h>

void hir_param_print (p_hir_param p_param) {
    assert(p_param);
    hir_exp_print(p_param->p_exp);
}

void hir_param_list_print(p_hir_param_list p_param_list) {
    assert(p_param_list);
    bool is_first = true;
    p_list_head p_node;
    list_for_each(p_node, &p_param_list->param) {
        if (!is_first) printf(", ");
        else is_first = false;

        p_hir_param p_param = list_entry(p_node, hir_param, node);
        hir_param_print(p_param);
    }
}