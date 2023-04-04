#ifndef __HIR_EXP__
#define __HIR_EXP__

#include <hir.h>

enum hir_exp_op {
    hir_exp_op_dot,
    hir_exp_op_assign,

    hir_exp_op_bool_or,
    hir_exp_op_bool_and,

    hir_exp_op_eq, hir_exp_op_neq,
    hir_exp_op_l, hir_exp_op_leq, hir_exp_op_g, hir_exp_op_geq,

    hir_exp_op_add, hir_exp_op_sub,
    hir_exp_op_mul, hir_exp_op_div, hir_exp_op_mod,

    hir_exp_op_bool_not, hir_exp_op_minus,

    hir_exp_op_arr,

    hir_exp_op_int2float,
    hir_exp_op_float2int,
};
struct hir_exp {
    enum {
        hir_exp_exec,
        hir_exp_call,
        hir_exp_id,
        hir_exp_num,
    } kind;
    union {
        struct {
            p_hir_exp p_src_1, p_src_2;
            hir_exp_op op;
        }; // exec 2
        struct {
            p_symbol_sym p_sym;
            p_hir_param_list p_param_list;
        }; // call id
        union {
            INTCONST_t intconst; // int
            FLOATCONST_t floatconst; // float
        };
    };
    bool is_basic;
    union {
        basic_type basic;
        p_symbol_type p_type;
    };
    bool syntax_const_exp;
};

#endif