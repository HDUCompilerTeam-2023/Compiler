#ifndef __IR_PARAM__
#define __IR_PARAM__
#include <ir.h>
struct ir_param {
    bool is_stack_ptr;
    bool is_in_mem;
    union {
        p_symbol_var p_vmem;
        p_ir_operand p_param;
    };
    list_head node;
};

#endif
