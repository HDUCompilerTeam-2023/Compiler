#ifndef __MIR_INSTR__
#define __MIR_INSTR__
#include <mir.h>

struct mir_instr{
    enum{
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
        mir_int2float_op,
        mir_float2int_op,
        mir_assign,
        // memory instr
        // mir_alloca,
        // mir_store,
        // mir_load,

        // other
        mir_call,
        mir_array,
        mir_ret,
        mir_br,
        mir_condbr,
    }irkind;
    union{
        p_mir_binary_instr p_mir_binary;
        p_mir_unary_instr p_mir_unary;
        p_mir_call_instr p_mir_call;
        p_mir_array_instr p_mir_array;
        p_mir_ret_instr p_mir_ret;
        p_mir_br_instr p_mir_br;
        p_mir_condbr_instr p_mir_condbr;
    };

    list_head node; // 下一条指令
};

struct mir_binary_instr{
    p_mir_operand src1, src2;
    p_mir_symbol des;
};

struct mir_unary_instr{
    p_mir_operand src;
    p_mir_symbol des;
};

struct mir_call_instr{
    p_mir_symbol p_func;
    p_mir_param_list p_param_list;
    p_mir_symbol des;
};

struct mir_array_instr{
    p_mir_symbol p_array;
    p_mir_operand p_offest;
    p_mir_symbol des;
};

struct mir_ret_instr{
    p_mir_operand p_ret;//  NULL 时返回 void
};

struct mir_br_instr{
    p_mir_instr p_target;
};

struct mir_condbr_instr{
    p_mir_instr p_target_true, p_target_false;
    p_mir_operand p_cond;
};

// 分配 des 的类型 变量给 des
// struct mir_alloca_instr{
//     p_mir_symbol des;
// };

// struct mir_load_instr{
//     p_mir_symbol src;
//     p_mir_symbol des;
// };

// struct mir_store_instr{
//     p_mir_symbol src;
//     p_mir_symbol des;
// };

#endif