#ifndef __IR_VREG__
#define __IR_VREG__

#include <ir.h>
struct ir_vreg {
    p_symbol_type p_type;
    size_t id;

    union {
        p_ir_basic_block p_bb_def;
        p_ir_instr p_instr_def;
    };
    bool is_bb_param;

    size_t reg_id;
    bool if_cond;  // 是否是条件跳转的条件
    list_head node;
};

#endif
