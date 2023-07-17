#include <ir_opt/lir_gen/share_trans.h>

#include <ir_gen.h>
#include <program/def.h>
#include <symbol_gen.h>

static inline void gep_trans(p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_symbol_func p_func) {
    p_ir_gep_instr p_gep_instr = &p_instr->ir_gep;
    size_t size = symbol_type_get_size(p_gep_instr->p_des->p_type);
    size *= basic_type_get_size(p_gep_instr->p_des->p_type->basic);
    assert(ir_operand_get_basic_type(p_gep_instr->p_offset) == type_i32);

    p_ir_vreg p_mul_des = ir_vreg_gen(symbol_type_var_gen(type_i32));
    p_ir_instr p_mul = ir_binary_instr_gen(ir_mul_op, ir_operand_copy(p_gep_instr->p_offset), ir_operand_int_gen(size), p_mul_des);
    symbol_func_vreg_add(p_func, p_mul_des);
    ir_instr_add_prev(p_mul, p_instr);

    p_ir_vreg p_add_des = p_gep_instr->p_des;
    ir_instr_reset_binary(p_instr, ir_add_op, ir_operand_copy(p_gep_instr->p_addr), ir_operand_vreg_gen(p_mul_des), p_add_des);
}

static void binary_trans(p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_binary_instr p_binary_instr = &p_instr->ir_binary;
    switch (p_binary_instr->op) {
    case ir_mod_op:
        assert(ir_operand_get_basic_type(p_binary_instr->p_src1) == type_i32);
        assert(ir_operand_get_basic_type(p_binary_instr->p_src2) == type_i32);

        p_ir_vreg p_div_des = ir_vreg_gen(symbol_type_var_gen(type_i32));
        p_ir_instr p_div = ir_binary_instr_gen(ir_div_op, ir_operand_copy(p_binary_instr->p_src1), ir_operand_copy(p_binary_instr->p_src2), p_div_des);
        symbol_func_vreg_add(p_func, p_div_des);
        ir_instr_add_prev(p_div, p_instr);

        p_ir_vreg p_mul_des = ir_vreg_gen(symbol_type_var_gen(type_i32));
        p_ir_instr p_mul = ir_binary_instr_gen(ir_mul_op, ir_operand_vreg_gen(p_div_des), ir_operand_copy(p_binary_instr->p_src2), p_mul_des);
        symbol_func_vreg_add(p_func, p_mul_des);
        ir_instr_add_prev(p_mul, p_instr);

        p_ir_vreg p_sub_des = p_binary_instr->p_des;
        ir_instr_reset_binary(p_instr, ir_sub_op, ir_operand_copy(p_binary_instr->p_src1), ir_operand_vreg_gen(p_mul_des), p_sub_des);
        break;
    case ir_add_op:
    case ir_sub_op:
    case ir_mul_op:
    case ir_div_op:
    case ir_l_op:
    case ir_leq_op:
    case ir_g_op:
    case ir_geq_op:
    case ir_eq_op:
    case ir_neq_op:
        break;
    }
}

static void unary_trans(p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_unary_instr p_unary_instr = &p_instr->ir_unary;
    switch (p_unary_instr->op) {
    case ir_minus_op:
    case ir_val_assign:
    case ir_f2i_op:
    case ir_i2f_op:
        break;
    }
}

static inline void deal_instr(p_ir_instr p_instr, p_ir_basic_block p_basic_block, p_symbol_func p_func) {
    switch (p_instr->irkind) {
    case ir_binary:
        binary_trans(p_instr, p_func);
        break;
    case ir_unary:
        unary_trans(p_instr, p_func);
        break;
    case ir_gep:
        gep_trans(p_instr, p_basic_block, p_func);
        break;
    case ir_store:
    case ir_load:
    case ir_call:
        break;
    }
}

static inline void share_lir_func_trans(p_symbol_func p_func) {
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_list_head p_instr_node, p_instr_node_next;
        list_for_each_safe(p_instr_node, p_instr_node_next, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            deal_instr(p_instr, p_basic_block, p_func);
        }
    }
    symbol_func_set_block_id(p_func);
}

void share_lir_trans_pass(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        share_lir_func_trans(p_func);
    }
}