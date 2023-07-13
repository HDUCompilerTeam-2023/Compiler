#ifndef __IR_INSTR__
#define __IR_INSTR__
#include <ir.h>
enum ir_binary_op {
    // binary instr
    ir_add_op,
    ir_sub_op,
    ir_mul_op,
    ir_div_op,
    ir_mod_op,
    ir_eq_op,
    ir_neq_op,
    ir_l_op,
    ir_leq_op,
    ir_g_op,
    ir_geq_op,
};

enum ir_unary_op {
    // unary instr
    ir_minus_op,
    ir_val_assign,
    ir_i2f_op,
    ir_f2i_op,
};

enum ir_instr_type {
    // binary instr
    ir_binary,

    // unary instr
    ir_unary,

    // memory
    ir_gep,
    ir_store,
    ir_load,
    // func call
    ir_call,
};

struct ir_binary_instr {
    ir_binary_op op;
    p_ir_operand p_src1, p_src2;
    p_ir_vreg p_des;
};

struct ir_unary_instr {
    ir_unary_op op;
    p_ir_operand p_src;
    p_ir_vreg p_des;
};

struct ir_call_instr {
    p_symbol_func p_func;
    p_ir_param_list p_param_list;
    p_ir_instr p_first_store;
    p_ir_vreg p_des;
};

struct ir_gep_instr {
    p_ir_operand p_addr;
    p_ir_operand p_offset;
    p_ir_vreg p_des;
    bool is_element;
};
struct ir_load_instr {
    bool is_stack_ptr;
    p_ir_operand p_addr;
    p_ir_vreg p_des;
};
struct ir_store_instr {
    bool is_stack_ptr;
    p_ir_operand p_addr;
    p_ir_operand p_src;
    p_ir_param p_call_param;
};

struct ir_instr {
    ir_instr_type irkind;
    union {
        ir_binary_instr ir_binary;
        ir_unary_instr ir_unary;
        ir_call_instr ir_call;
        ir_load_instr ir_load;
        ir_gep_instr ir_gep;
        ir_store_instr ir_store;
    };
    size_t instr_id;
    p_ir_bb_phi_list p_live_in;
    p_ir_bb_phi_list p_live_out;
    list_head node; // 下一条指令
};

#endif
