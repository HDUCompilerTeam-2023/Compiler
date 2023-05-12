#ifndef __MIR_INSTR__
#define __MIR_INSTR__
#include <mir.h>
enum mir_instr_type {
    // binary instr
    mir_add_op,
    mir_sub_op,
    mir_mul_op,
    mir_div_op,
    mir_mod_op,
    mir_and_op,
    mir_or_op,
    mir_eq_op,
    mir_neq_op,
    mir_l_op,
    mir_leq_op,
    mir_g_op,
    mir_geq_op,

    // unary instr
    mir_minus_op,
    mir_not_op,
    mir_val_assign,

    // memory
    mir_addr,
    mir_store,
    mir_load,
    // func call
    mir_call,
    mir_ret,
    // jump
    mir_br,
    mir_condbr,
};

struct mir_binary_instr {
    p_mir_operand p_src1, p_src2;
    p_mir_vreg p_des;
};

struct mir_unary_instr {
    p_mir_operand p_src;
    p_mir_vreg p_des;
};

struct mir_call_instr {
    p_mir_func p_func;
    p_mir_param_list p_param_list;
    p_mir_vreg p_des;
};

struct mir_load_instr {
    p_mir_operand p_addr;
    p_mir_operand p_offset;
    p_mir_vreg p_des;
};
struct mir_addr_instr {
    p_mir_vmem p_vmem;
    p_mir_vreg p_des;
};
struct mir_store_instr {
    p_mir_operand p_addr;
    p_mir_operand p_offset;
    p_mir_operand p_src;
};
struct mir_ret_instr {
    p_mir_operand p_ret; //  NULL 时返回 void
};

struct mir_br_instr {
    p_mir_basic_block_call p_target;
};

struct mir_condbr_instr {
    p_mir_basic_block_call p_target_true, p_target_false;
    p_mir_operand p_cond;
};

struct mir_instr {
    mir_instr_type irkind;
    union {
        mir_binary_instr mir_binary;
        mir_unary_instr mir_unary;
        mir_call_instr mir_call;
        mir_addr_instr mir_addr;
        mir_load_instr mir_load;
        mir_store_instr mir_store;
        mir_ret_instr mir_ret;
        mir_br_instr mir_br;
        mir_condbr_instr mir_condbr;
    };

    list_head node; // 下一条指令
};

#endif
