#ifndef __HIR__
#define __HIR__

#include <util.h>

typedef uint64_t INTCONST_t;
typedef double FLOATCONST_t;
typedef char *ID_t;

typedef struct hir_program hir_program, *p_hir_program;
typedef struct hir_param hir_param, *p_hir_param;
typedef struct hir_param_list hir_param_list, *p_hir_param_list;
typedef struct hir_func hir_func, *p_hir_func;
typedef struct hir_block hir_block, *p_hir_block;
typedef struct hir_stmt hir_stmt, *p_hir_stmt;
typedef struct hir_exp hir_exp, *p_hir_exp;
typedef enum hir_exp_op hir_exp_op;

#include <hir/type.h>
#include <hir/symbol.h>

struct hir_program {
    // list_head init;
    p_symbol_store pss;
    list_head func;
};

struct hir_block {
    list_head stmt;
};

struct hir_param_list {
    list_head param;
};
struct hir_func {
    p_hir_block p_block;
    p_symbol_sym p_sym;

    list_head node;
};

enum hir_exp_op {
    hir_exp_op_dot,
    hir_exp_op_assign,

    hir_exp_op_bool_or,
    hir_exp_op_bool_and,

    hir_exp_op_eq, hir_exp_op_neq,
    hir_exp_op_l, hir_exp_op_leq, hir_exp_op_g, hir_exp_op_geq,

    hir_exp_op_add, hir_exp_op_sub,
    hir_exp_op_mul, hir_exp_op_div, hir_exp_op_mod,

    // hir_exp_op_fadd, hir_exp_op_fsub,
    // hir_exp_op_fmul, hir_exp_op_fdiv, hir_exp_op_fmod,

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

struct hir_param {
    p_hir_exp p_exp;
    list_head node;
};

struct hir_stmt {
    enum {
        hir_stmt_return, hir_stmt_exp, // p_exp (can null)
        hir_stmt_block, // p_block
        hir_stmt_if_else, // p_exp p_stmt_1 p_stmt_2
        hir_stmt_if, hir_stmt_while, // p_exp p_stmt_1
        hir_stmt_break, hir_stmt_continue, // null
    } type;

    union {
        struct {
            p_hir_exp p_exp;
            p_hir_stmt p_stmt_1, p_stmt_2;
        };
        p_hir_block p_block;
        void *null;
    };

    list_head node;
};

p_hir_stmt hir_stmt_return_gen(p_hir_exp p_exp);
p_hir_stmt hir_stmt_exp_gen(p_hir_exp p_exp);
p_hir_stmt hir_stmt_break_gen(void);
p_hir_stmt hir_stmt_continue_gen(void);
p_hir_stmt hir_stmt_if_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1);
p_hir_stmt hir_stmt_if_else_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1, p_hir_stmt p_stmt_2);
p_hir_stmt hir_stmt_while_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1);
p_hir_stmt hir_stmt_do_while_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1);
p_hir_stmt hir_stmt_block_gen(p_hir_block p_block);
void hir_stmt_drop(p_hir_stmt p_stmt);

p_hir_exp hir_exp_dot_gen(p_hir_exp p_src_1, p_hir_exp p_src_2);
p_hir_exp hir_exp_assign_gen(p_hir_exp lval, p_hir_exp rval);
p_hir_exp hir_exp_exec_gen(hir_exp_op op, p_hir_exp p_src_1, p_hir_exp p_src_2);
p_hir_exp hir_exp_lexec_gen(hir_exp_op op, p_hir_exp p_src_1, p_hir_exp p_src_2);
p_hir_exp hir_exp_uexec_gen(hir_exp_op op, p_hir_exp p_src_1);
p_hir_exp hir_exp_call_gen(p_symbol_sym, p_hir_param_list p_param_list);
p_hir_exp hir_exp_id_gen(p_symbol_sym p_sym);
p_hir_exp hir_exp_arr_gen(p_hir_exp p_arrary, p_hir_exp p_index);
p_hir_exp hir_exp_int_gen(INTCONST_t num);
p_hir_exp hir_exp_float_gen(FLOATCONST_t num);
void hir_exp_drop(p_hir_exp p_exp);

p_hir_param_list hir_param_list_init(void);
p_hir_param_list hir_param_list_add(p_hir_param_list p_head, p_hir_exp p_exp);
void hir_param_list_drop(p_hir_param_list p_param_list);

p_hir_block hir_block_gen(void);
p_hir_block hir_block_add(p_hir_block p_block, p_hir_stmt p_stmt);
void hir_block_drop(p_hir_block p_block);

p_hir_func hir_func_gen(p_symbol_sym p_sym, p_hir_block p_block);
void hir_func_drop(p_hir_func p_func);

p_hir_program hir_program_gen(void);
p_hir_program hir_program_add(p_hir_program p_program, p_hir_func p_func);
void hir_program_drop(p_hir_program p_program);

#endif