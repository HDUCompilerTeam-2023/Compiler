#include <ir_gen.h>
#include <ir_gen/param.h>

p_ir_param_list ir_param_list_init(void) {
    p_ir_param_list p_param_list = malloc(sizeof(*p_param_list));
    *p_param_list = (ir_param_list) {
        .param = list_head_init(&p_param_list->param)
    };
    return p_param_list;
}

p_ir_param_list ir_param_list_add(p_ir_param_list p_param_list, p_ir_operand p_param, bool is_stack_ptr) {
    p_ir_param p_ir_param = malloc(sizeof(*p_ir_param));
    *p_ir_param = (ir_param) {
        .p_param = p_param,
        .is_stack_ptr = is_stack_ptr,
        .node = list_head_init(&p_ir_param->node),
    };

    list_add_prev(&p_ir_param->node, &p_param_list->param);
    return p_param_list;
}

void ir_param_list_drop(p_ir_param_list p_param_list) {
    assert(p_param_list);
    while (!list_head_alone(&p_param_list->param)) {
        p_ir_param p_param = list_entry(p_param_list->param.p_next, ir_param, node);
        ir_operand_drop(p_param->p_param);
        list_del(&p_param->node);
        free(p_param);
    }
    free(p_param_list);
}
