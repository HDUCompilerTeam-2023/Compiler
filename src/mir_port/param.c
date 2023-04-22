
#include <mir_port/param.h>
#include <mir/param.h>


p_mir_operand mir_param_get_operand(p_mir_param p_param)
{
    return p_param->p_param;
}

p_list_head mir_param_list_get_head(p_mir_param_list p_mir_param_list)
{
    return &p_mir_param_list->param;
}