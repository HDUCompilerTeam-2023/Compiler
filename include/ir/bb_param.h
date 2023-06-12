#ifndef __IR_BB_PARAM__
#define __IR_BB_PARAM__

#include <ir.h>
struct ir_bb_param {
    p_ir_operand p_bb_param;
    list_head node;
};

struct ir_bb_param_list {
    list_head bb_param;
};

struct ir_bb_phi {
    p_ir_vreg p_bb_phi;
    list_head node;
};

struct ir_bb_phi_list {
    list_head bb_phi;
};

#endif
