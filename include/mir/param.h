#ifndef __MIR_PARAM__
#define __MIR_PARAM__
#include <mir.h>
struct mir_param{
    p_mir_operand p_param;
    list_head node;
};

struct mir_param_list{
    list_head param;
};

#endif