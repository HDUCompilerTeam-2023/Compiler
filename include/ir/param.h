#ifndef __IR_PARAM__
#define __IR_PARAM__
#include <ir.h>
struct ir_param {
    p_ir_operand p_param;
    list_head node;
};

struct ir_param_list {
    list_head param;
};

#endif
