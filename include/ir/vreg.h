#ifndef __IR_VREG__
#define __IR_VREG__

#include <ir.h>
#include <ir_opt/scev.h>

typedef struct scevexp scevexp, *p_scevexp;

struct scevexp {
    p_ir_vreg p_des;
    bool is_scev1, is_scev2;
    union {
        p_scevexp p_scev1;
        p_ir_operand p_operand1;
        p_basic_var_info p_var_info;
    };
    union {
        p_scevexp p_scev2;
        p_ir_operand p_operand2;
    };
};
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

    bool is_loop_inv; // 是否是循环不变量
    enum {
        scev_add,
        scev_sub,
        scev_mul,
        scev_div,
        scev_mod,
        scev_unknown,
        scev_basic,
    } scev_kind;
    p_scevexp p_scevexp;

    void *p_info; // 额外信息（目前只用在图着色）
    list_head node;
};

struct ir_vreg_list_node {
    p_ir_vreg p_vreg;
    size_t next_use;
    list_head node;
};
struct ir_vreg_list {
    list_head vreg_list;
};
#endif
