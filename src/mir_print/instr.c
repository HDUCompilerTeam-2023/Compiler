#include <mir/basic_block.h>
#include <mir/func.h>
#include <mir/instr.h>
#include <mir_print.h>
#include <stdio.h>

#include <symbol/var.h>
#include <symbol/func.h>
void mir_instr_print(p_mir_instr p_instr) {
    assert(p_instr);
    printf("    ");
    switch (p_instr->irkind) {
    case mir_binary:
        mir_binary_instr_print(&p_instr->mir_binary);
        break;
    case mir_unary:
        mir_unary_instr_print(&p_instr->mir_unary);
        break;
    case mir_call:
        mir_call_instr_print(&p_instr->mir_call);
        break;
    case mir_load:
        mir_load_instr_print(&p_instr->mir_load);
        break;
    case mir_store:
        mir_store_instr_print(&p_instr->mir_store);
    }
    printf("\n");
}

void mir_binary_instr_print(p_mir_binary_instr p_instr) {
    mir_vreg_print(p_instr->p_des);
    printf(" = ");
    mir_operand_print(p_instr->p_src1);
    switch (p_instr->op) {
    case mir_add_op:
        printf("+ ");
        break;
    case mir_sub_op:
        printf("- ");
        break;
    case mir_mul_op:
        printf("* ");
        break;
    case mir_div_op:
        printf("/ ");
        break;
    case mir_mod_op:
        printf("%% ");
        break;
    case mir_and_op:
        printf("&& ");
        break;
    case mir_or_op:
        printf("|| ");
        break;
    case mir_eq_op:
        printf("== ");
        break;
    case mir_neq_op:
        printf("!= ");
        break;
    case mir_l_op:
        printf("< ");
        break;
    case mir_leq_op:
        printf("<= ");
        break;
    case mir_g_op:
        printf("> ");
        break;
    case mir_geq_op:
        printf(">= ");
        break;
    default:
        assert(0);
    }
    mir_operand_print(p_instr->p_src2);
}

void mir_unary_instr_print(p_mir_unary_instr p_instr) {
    mir_vreg_print(p_instr->p_des);
    printf(" = ");
    switch (p_instr->op) {
    case mir_minus_op:
        printf("- ");
        break;
    case mir_not_op:
        printf("! ");
        break;
    case mir_val_assign:
        break;
    default:
        assert(0);
    }
    mir_operand_print(p_instr->p_src);
}

void mir_call_instr_print(p_mir_call_instr p_instr) {
    mir_vreg_print(p_instr->p_des);
    printf(" = ");
    printf("@%s", p_instr->p_func->name);
    mir_param_list_print(p_instr->p_param_list);
}

void mir_load_instr_print(p_mir_load_instr p_instr) {
    mir_vreg_print(p_instr->p_des);
    printf(" = load ");
    mir_operand_print(p_instr->p_addr);
    if (p_instr->p_offset) {
        mir_operand_print(p_instr->p_offset);
    }
}

void mir_store_instr_print(p_mir_store_instr p_instr) {
    printf("store ");
    mir_operand_print(p_instr->p_addr);
    if (p_instr->p_offset) {
        mir_operand_print(p_instr->p_offset);
    }
    printf("= ");
    mir_operand_print(p_instr->p_src);
}
