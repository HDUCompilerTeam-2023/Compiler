#ifndef __MIR_BB_PARAM__
#define __MIR_BB_PARAM__

#include <mir.h>
struct mir_bb_param {
    p_mir_operand p_bb_param;
    list_head node;
};

struct mir_bb_param_list {
    list_head bb_param;
};

struct mir_bb_phi {
    p_mir_vreg p_bb_phi;
    list_head node;
};

struct mir_bb_phi_list {
    list_head bb_phi;
};

#endif
