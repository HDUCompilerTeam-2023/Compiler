#include <mir_gen.h>
#include <mir_gen/param.h>

p_mir_param_list mir_param_list_init(void)
{
    p_mir_param_list p_param_list = malloc(sizeof(*p_param_list));
    *p_param_list = (mir_param_list){
        .param = list_head_init(&p_param_list->param)
    };
    return p_param_list;
}

p_mir_param_list mir_param_list_add(p_mir_param_list p_param_list, p_mir_operand p_param)
{
    p_mir_param p_ir_param = malloc(sizeof(*p_ir_param));
    *p_ir_param = (mir_param){
        .p_param = p_param,
        .node = list_head_init(&p_ir_param->node),
    };

    list_add_prev(&p_ir_param->node, &p_param_list->param);
    return p_param_list;
}

void mir_param_list_drop(p_mir_param_list p_param_list)
{
    assert(p_param_list);
    while (!list_head_alone(&p_param_list->param)) {
        p_mir_param p_param = list_entry(p_param_list->param.p_next, mir_param, node);
        mir_operand_drop(p_param->p_param);
        list_del(&p_param->node);
        free(p_param);
    }
    free(p_param_list);
}