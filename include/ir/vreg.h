#ifndef __IR_VREG__
#define __IR_VREG__

#include <ir.h>
struct ir_vreg {
    p_symbol_type p_type;
    size_t id;

    union {
        p_ir_bb_phi p_bb_phi;
        p_ir_instr p_instr_def;
        p_symbol_func p_func;
    };
    enum {
        instr_def,
        bb_phi_def,
        func_param_def,
    } def_type;

    list_head use_list;

    size_t reg_id;
    bool if_float; // 是否在浮点寄存器
    bool if_cond; // 是否是条件跳转的条件

    void *p_info; // 额外信息（目前只用在图着色）
    list_head node;
};

struct ir_vreg_list_node {
    p_ir_vreg p_vreg;
    list_head node;
};
struct ir_vreg_list {
    list_head vreg_list;
};
#endif
