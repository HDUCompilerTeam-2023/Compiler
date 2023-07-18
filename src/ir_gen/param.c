#include <ir_gen.h>
#include <ir_gen/param.h>
p_ir_param ir_param_gen(p_ir_operand p_param) {
    p_ir_param p_ir_param = malloc(sizeof(*p_ir_param));
    *p_ir_param = (ir_param) {
        .p_param = p_param,
        .is_in_mem = false,
        .node = list_head_init(&p_ir_param->node),
    };
    return p_ir_param;
}
void ir_param_set_vmem(p_ir_param p_param, p_symbol_var p_vmem) {
    p_param->is_in_mem = true;
    ir_operand_drop(p_param->p_param);
    p_param->p_vmem = p_vmem;
}