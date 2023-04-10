#include <hir_gen/param.h>

#include <hir_gen.h>

p_hir_param_list hir_param_list_init(void) {
    p_hir_param_list p_param_list = malloc(sizeof(*p_param_list));
    *p_param_list = (hir_param_list) {
        .param = list_head_init(&p_param_list->param),
    };
    return p_param_list;
}
p_hir_param_list hir_param_list_add(p_hir_param_list p_param_list, p_hir_exp p_exp) {
    p_hir_param p_param = malloc(sizeof(*p_param));
    *p_param = (hir_param) {
        .p_exp = p_exp,
        .node = list_head_init(&p_param->node),
    };

    list_add_prev(&p_param->node, &p_param_list->param);
    return p_param_list;
}
void hir_param_drop (p_hir_param p_param) {
    assert(p_param);
    hir_exp_drop(p_param->p_exp);
    free(p_param);
}

void hir_param_list_drop(p_hir_param_list p_param_list) {
    assert(p_param_list);
    while(!list_head_alone(&p_param_list->param)) {
        p_hir_param p_param = list_entry(p_param_list->param.p_next, hir_param, node);
        list_del(&p_param->node);
        hir_param_drop(p_param);
    }
    free(p_param_list);
}