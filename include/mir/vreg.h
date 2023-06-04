#ifndef __MIR_VREG__
#define __MIR_VREG__

#include <mir.h>
struct mir_vreg {
    p_symbol_type p_type;
    size_t id;

    union {
        p_mir_basic_block p_bb_def;
        p_mir_instr p_instr_def;
    };
    bool is_bb_param;

    list_head node;
};

#endif
