#ifndef __HIR_EXP__
#define __HIR_EXP__

#include <hir.h>
#include <mir.h>

enum hir_exp_logic_op {
    hir_exp_op_bool_or,
    hir_exp_op_bool_and,
};
enum hir_exp_ulogic_op {
    hir_exp_op_bool_not,
};
enum hir_exp_binary_op {
    hir_exp_op_eq,
    hir_exp_op_neq,
    hir_exp_op_l,
    hir_exp_op_leq,
    hir_exp_op_g,
    hir_exp_op_geq,

    hir_exp_op_add,
    hir_exp_op_sub,
    hir_exp_op_mul,
    hir_exp_op_div,
    hir_exp_op_mod,
};
enum hir_exp_unary_op {
    hir_exp_op_minus,
};
struct hir_exp {
    enum {
        hir_exp_binary,
        hir_exp_unary,
        hir_exp_logic,
        hir_exp_ulogic,
        hir_exp_use,
        hir_exp_call,
        hir_exp_ptr,
        hir_exp_gep,
        hir_exp_load,
        hir_exp_num,
    } kind;
    union {
        struct {
            p_hir_exp p_src_1, p_src_2;
            hir_exp_binary_op b_op;
        }; // binary
        struct {
            p_hir_exp p_src;
            hir_exp_unary_op u_op;
        }; // unary
        struct {
            p_hir_exp p_bool_1, p_bool_2;
            hir_exp_logic_op l_op;
        }; // logic
        struct {
            p_hir_exp p_bool;
            hir_exp_ulogic_op ul_op;
        }; // ulogic
        p_hir_exp p_exp; // use
        struct {
            p_symbol_func p_func;
            p_hir_param_list p_param_list;
        }; // call
        p_symbol_var p_var; // ptr
        struct {
            bool is_element;
            p_hir_exp p_addr;
            p_hir_exp p_offset;
        }; // gep
        p_hir_exp p_ptr; // load
        union {
            INTCONST_t intconst; // int
            FLOATCONST_t floatconst; // float
            p_symbol_str p_str; // str
        }; // null
    };
    p_symbol_type p_type;

    p_mir_vreg p_des;
};

#endif
