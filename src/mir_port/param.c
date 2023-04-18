#include <mir_port/param.h>
#include <mir/param.h>


p_mir_operand mir_param_get_operand(p_mir_param p_param)
{
    return p_param->p_param;
}
