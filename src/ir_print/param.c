#include <ir/param.h>
#include <ir_print.h>

#include <stdio.h>
#include <symbol_print.h>
void ir_param_print(p_ir_param p_param) {
    if (!p_param->is_in_mem)
        ir_operand_print(p_param->p_param);
    else
        symbol_name_print(p_param->p_vmem);
}
