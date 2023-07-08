#ifndef __AST_EXP__
#define __AST_EXP__

#include <ast.h>
#include <ir.h>

enum ast_exp_logic_op {
    ast_exp_op_bool_or,
    ast_exp_op_bool_and,
};
enum ast_exp_ulogic_op {
    ast_exp_op_bool_not,
};
enum ast_exp_relational_op {
    ast_exp_op_eq,
    ast_exp_op_neq,
    ast_exp_op_l,
    ast_exp_op_leq,
    ast_exp_op_g,
    ast_exp_op_geq,
};
enum ast_exp_binary_op {
    ast_exp_op_add,
    ast_exp_op_sub,
    ast_exp_op_mul,
    ast_exp_op_div,
    ast_exp_op_mod,
};
enum ast_exp_unary_op {
    ast_exp_op_minus,
    ast_exp_op_i2f,
    ast_exp_op_f2i,
};
struct ast_exp {
    enum {
        ast_exp_binary,
        ast_exp_relational,
        ast_exp_unary,
        ast_exp_logic,
        ast_exp_ulogic,
        ast_exp_use,
        ast_exp_call,
        ast_exp_ptr,
        ast_exp_gep,
        ast_exp_load,
        ast_exp_num,
    } kind;
    union {
        struct {
            p_ast_exp p_rsrc_1, p_rsrc_2;
            ast_exp_relational_op r_op;
        }; // relational
        struct {
            p_ast_exp p_src_1, p_src_2;
            ast_exp_binary_op b_op;
        }; // binary
        struct {
            p_ast_exp p_src;
            ast_exp_unary_op u_op;
        }; // unary
        struct {
            p_ast_exp p_bool_1, p_bool_2;
            ast_exp_logic_op l_op;
        }; // logic
        struct {
            p_ast_exp p_bool;
            ast_exp_ulogic_op ul_op;
        }; // ulogic
        p_ast_exp p_exp; // use
        struct {
            p_symbol_func p_func;
            p_ast_param_list p_param_list;
        }; // call
        p_symbol_var p_var; // ptr
        struct {
            bool is_element;
            bool is_stack_for_gep;
            p_ast_exp p_addr;
            p_ast_exp p_offset;
        }; // gep
        struct {
            bool is_stack;
            p_ast_exp p_ptr;
        }; // load
        union {
            I32CONST_t i32const; // int
            F32CONST_t f32const; // float
            p_symbol_str p_str; // str
        }; // null
    };
    p_symbol_type p_type;

    p_ir_vreg p_des;
};

#endif
