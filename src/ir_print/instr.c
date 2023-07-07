#include <ir/basic_block.h>
#include <ir/instr.h>
#include <ir_print.h>
#include <stdio.h>
#include <symbol_print.h>

#include <symbol/func.h>
#include <symbol/var.h>
void ir_instr_print(p_ir_instr p_instr) {
    assert(p_instr);
    printf("%ld:        ", p_instr->instr_id);
    switch (p_instr->irkind) {
    case ir_binary:
        ir_binary_instr_print(&p_instr->ir_binary);
        break;
    case ir_unary:
        ir_unary_instr_print(&p_instr->ir_unary);
        break;
    case ir_call:
        ir_call_instr_print(&p_instr->ir_call);
        break;
    case ir_gep:
        ir_gep_instr_print(&p_instr->ir_gep);
        break;
    case ir_load:
        ir_load_instr_print(&p_instr->ir_load);
        break;
    case ir_store:
        ir_store_instr_print(&p_instr->ir_store);
    }
    printf("\n");
}

void ir_binary_instr_print(p_ir_binary_instr p_instr) {
    ir_vreg_print(p_instr->p_des);
    printf(" = ");
    ir_operand_print(p_instr->p_src1);
    switch (p_instr->op) {
    case ir_add_op:
        printf(" + ");
        break;
    case ir_sub_op:
        printf(" - ");
        break;
    case ir_mul_op:
        printf(" * ");
        break;
    case ir_div_op:
        printf(" / ");
        break;
    case ir_mod_op:
        printf(" %% ");
        break;
    case ir_eq_op:
        printf(" == ");
        break;
    case ir_neq_op:
        printf(" != ");
        break;
    case ir_l_op:
        printf(" < ");
        break;
    case ir_leq_op:
        printf(" <= ");
        break;
    case ir_g_op:
        printf(" > ");
        break;
    case ir_geq_op:
        printf(" >= ");
        break;
    }
    ir_operand_print(p_instr->p_src2);
}

void ir_unary_instr_print(p_ir_unary_instr p_instr) {
    ir_vreg_print(p_instr->p_des);
    printf(" = ");
    switch (p_instr->op) {
    case ir_minus_op:
        printf("-");
        break;
    case ir_val_assign:
        break;
    case ir_i2f_op:
        printf("i2f ");
        break;
    case ir_f2i_op:
        printf("f2i ");
        break;
    }
    ir_operand_print(p_instr->p_src);
}

void ir_call_instr_print(p_ir_call_instr p_instr) {
    if (p_instr->p_des) {
        ir_vreg_print(p_instr->p_des);
        printf(" = ");
    }
    symbol_func_name_print(p_instr->p_func);
    ir_param_list_print(p_instr->p_param_list);
}

void ir_gep_instr_print(p_ir_gep_instr p_instr) {
    ir_vreg_print(p_instr->p_des);
    printf(" = getelementptr ");
    ir_operand_print(p_instr->p_addr);
    if (p_instr->is_element)
        printf(" i32 0");
    printf(" ");
    ir_operand_print(p_instr->p_offset);
}

void ir_load_instr_print(p_ir_load_instr p_instr) {
    ir_vreg_print(p_instr->p_des);
    printf(" = load ");
    ir_operand_print(p_instr->p_addr);
}

void ir_store_instr_print(p_ir_store_instr p_instr) {
    printf("store ");
    ir_operand_print(p_instr->p_addr);
    printf(" = ");
    ir_operand_print(p_instr->p_src);
}
